#include "core/algorithms/Strings.h"
#include "core/containers/Array.h"
#include "core/containers/HashMap.h"
#include "core/containers/Span.h"
#include "core/io/file_system/FileSystem.h"
#include "core/io/serialization/ArrayBuffer.h"
#include "core/io/serialization/OutputStream.h"
#include "core/logging/Logger.h"

#include "assets/materials/Shader.h"

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

Core::LogCategory ShaderCompiler("Shader Compiler");
Core::LogCategory Glslang("glslang");

struct SourceFiles {
    std::filesystem::path vertexFile;
    std::filesystem::path fragmentFile;
    std::filesystem::path semanticsFile;
    std::vector<std::filesystem::path> includeDirectories;
};

void printLog(const char* log, Core::LogLevel level) {
    Core::Array<std::string_view> lines = Core::Algorithms::String::SplitLines(log);
    for(auto line : lines) {
        Core::Log::Log(Glslang, level, line);
    }
}

Core::StatusOr<std::unique_ptr<glslang::TShader>> compileShader(EShLanguage stage,
                                                                const std::filesystem::path& sourceFile,
                                                                DirStackFileIncluder& includer) {
    ASSIGN_OR_RETURN(std::string sourceText, Core::IO::ReadTextFile(sourceFile));

    std::unique_ptr<glslang::TShader> shader = std::make_unique<glslang::TShader>(stage);

    const char* sourceCString = sourceText.c_str();

    std::string nameString  = sourceFile.string();
    const char* nameCString = nameString.c_str();

    shader->setStringsWithLengthsAndNames(&sourceCString, nullptr, &nameCString, 1);
    shader->setEnvInput(glslang::EShSource::EShSourceGlsl, stage, glslang::EShClient::EShClientVulkan, 100);
    shader->setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader->setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_5);

    const int version            = 110;
    const bool forwardCompatible = false;

    bool compiledSuccessfully = shader->parse(
          &glslang::DefaultTBuiltInResource, version, forwardCompatible, EShMessages::EShMsgDefault, includer);


    if(!compiledSuccessfully) {
        printLog(shader->getInfoLog(), Core::LogLevel::Error);
        printLog(shader->getInfoDebugLog(), Core::LogLevel::Debug);

        return Core::Status::Error("Compilation failed");
    } else {
        printLog(shader->getInfoLog(), Core::LogLevel::Info);
        printLog(shader->getInfoDebugLog(), Core::LogLevel::Debug);

        return shader;
    }
}

Core::Array<std::byte> dumpShader(const glslang::TProgram& program, EShLanguage stage) {
    std::vector<uint32_t> spirv;
    spv::SpvBuildLogger logger;

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger);

    std::string logs = logger.getAllMessages();
    for(const auto& line : Core::Algorithms::String::SplitLines(logs)) {
        Core::Log::Info(Glslang, line);
    }

    Core::Span<std::byte> spirvSpan = Core::AsWritableBytes(Core::ToSpan(spirv));

    return Core::Array<std::byte>(spirvSpan);
}

Core::StatusOr<Assets::PropertyTypeName> ToPropertyTypeName(const glslang::TBasicType basicType) {
    switch(basicType) {
        case glslang::EbtFloat: return Assets::PropertyType::FLOAT_32;
        case glslang::EbtDouble: return Assets::PropertyType::FLOAT_64;
        case glslang::EbtInt16: return Assets::PropertyType::INT_16;
        case glslang::EbtInt: return Assets::PropertyType::INT_32;
        case glslang::EbtUint16: return Assets::PropertyType::UINT_16;
        case glslang::EbtUint: return Assets::PropertyType::UINT_32;
        default:
            return Core::Status::Error("Basic type '{}' is not a supported property type!",
                                       glslang::TType::getBasicString(basicType));
    }
}

struct Attribute {
    Assets::Property property;
    uint8_t count;
};

Core::StatusOr<Attribute> ToProperty(const glslang::TType& type) {
    uint8_t elementCount  = 1;
    uint8_t propertyCount = 1;
    if(type.isVector()) {
        elementCount = type.getVectorSize();
    } else if(type.isMatrix()) {
        // Matricies are column major, meaning there are 'column' count attributes, of size 'row'
        elementCount  = type.getMatrixRows();
        propertyCount = type.getMatrixCols();
    } else if(type.isArray()) {
        // Arrays always have one element per attribute, no matter their size
        propertyCount = type.getCumulativeArraySize();
    }

    ASSIGN_OR_RETURN(Assets::PropertyTypeName propertyType, ToPropertyTypeName(type.getBasicType()));

    return Attribute{.property =
                           {
                                 .type         = propertyType,
                                 .elementCount = elementCount,
                           },
                     .count = propertyCount};
}

uint32_t ToBinding(const glslang::TType& type) {
    const glslang::TQualifier& qualifier = type.getQualifier();
    return qualifier.hasBinding() ? qualifier.layoutBinding : 0;
}

Core::StatusOr<Assets::VertexInput> ToVertexInput(const glslang::TType& type,
                                                  Assets::VertexUsageName usage,
                                                  Assets::VertexInputRateType rate) {
    const glslang::TQualifier& qualifier = type.getQualifier();

    uint32_t location = qualifier.layoutLocation;
    ASSIGN_OR_RETURN(Attribute attribute, ToProperty(type));

    return Assets::VertexInput{.startLocation = location,
                               .locationCount = attribute.count,
                               .rate          = rate,
                               .property      = attribute.property,
                               .usage         = usage};
}

Core::StatusOr<Assets::ShaderUniformDescription>
ToUniformDescription(const glslang::TType& type, Core::HashMap<std::string, uint8_t> bufferOffsets) {
    if(type.getBasicType() == glslang::EbtSampler) {
        return Assets::ShaderUniformDescription(Assets::SamplerDescription());
    } else if(!type.isStruct()) {
        return Core::Status::Error("Unsupported uniform type: {}", type.getBasicString());
    }

    Assets::BufferDescription buffer;

    const glslang::TTypeList& members = *type.getStruct();
    for(const auto& t : members) {
        const glslang::TType& fieldType = *t.type;

        ASSIGN_OR_RETURN(Attribute property, ToProperty(fieldType));

        std::string fieldName = std::string(fieldType.getFieldName());

        buffer.properties[fieldName] = Assets::BufferProperty{
              .property   = property.property,
              .byteOffset = bufferOffsets[fieldName],
              .count      = 1,
        };
    }

    return Assets::ShaderUniformDescription(buffer);
}

Core::StatusOr<Assets::ShaderUniform>
ToShaderUniform(const glslang::TObjectReflection& reflection,
                Core::HashMap<std::string, Core::HashMap<std::string, uint8_t>> bufferOffsets) {
    const glslang::TType& type = *reflection.getType();

    uint32_t binding = ToBinding(type);

    Assets::PipelineStageType pipelineStage = Assets::PipelineStage::VERTEX;
    if(reflection.stages & EShLangVertexMask) {
        pipelineStage = Assets::PipelineStage::VERTEX;
    } else if(reflection.stages & EShLangFragmentMask) {
        pipelineStage = Assets::PipelineStage::FRAGMENT;
    }

    ASSIGN_OR_RETURN(Assets::ShaderUniformDescription description,
                     ToUniformDescription(type, bufferOffsets[reflection.name]));

    return Assets::ShaderUniform{.bindingIndex = binding, .stage = pipelineStage, .description = description};
}

Core::StatusOr<Assets::VertexUsageName> GetVertexUsage(const std::string& name, const nlohmann::json& mapping) {
    auto usageNode = mapping[name]["usage"];
    if(usageNode.is_null()) {
        return Core::Status::Error("Missing usage semantic for '{}'", name);
    } else if(!usageNode.is_string()) {
        return Core::Status::Error("Invalid usage semantic for '{}': {}", name, usageNode.dump());
    }

    std::string usage = usageNode.get<std::string>();
    if(usage == "position") {
        return Assets::VertexUsage::POSITION;
    } else if(usage == "texture") {
        return Assets::VertexUsage::TEXTURE;
    } else if(usage == "texture2") {
        return Assets::VertexUsage::TEXTURE2;
    } else if(usage == "normal") {
        return Assets::VertexUsage::NORMAL;
    } else if(usage == "colour") {
        return Assets::VertexUsage::COLOUR;
    } else if(usage == "transform") {
        return Assets::VertexUsage::TRANSFORM;
    } else {
        return Core::Status::Error("Unknown usage semantic for '{}': {}", name, usage);
    }
}

Core::StatusOr<Assets::VertexInputRateType> GetVertexRate(const std::string& name, const nlohmann::json& mapping) {
    auto rateNode = mapping[name]["rate"];
    if(rateNode.is_null()) {
        return Core::Status::Error("Missing rate semantic for '{}'", name);
    } else if(!rateNode.is_string()) {
        return Core::Status::Error("Invalid rate semantic for '{}': {}", name, rateNode.dump());
    }

    std::string rate = rateNode.get<std::string>();
    if(rate == "vertex") {
        return Assets::VertexInputRate::PER_VERTEX;
    } else if(rate == "instance") {
        return Assets::VertexInputRate::PER_INSTANCE;
    } else {
        return Core::Status::Error("Unknown rate semantic for '{}': {}", name, rate);
    }
}

Core::Status run(const SourceFiles& inputFiles, const std::filesystem::path& outputFile) {
    DirStackFileIncluder includer;
    for(const auto& path : inputFiles.includeDirectories) {
        includer.pushExternalLocalDirectory(path.string());
    }

    glslang::InitializeProcess();

    ASSIGN_OR_RETURN(std::unique_ptr<glslang::TShader> vertexShader,
                     compileShader(EShLanguage::EShLangVertex, inputFiles.vertexFile, includer));

    ASSIGN_OR_RETURN(std::unique_ptr<glslang::TShader> fragmentShader,
                     compileShader(EShLanguage::EShLangFragment, inputFiles.fragmentFile, includer));

    glslang::TProgram program;
    program.addShader(vertexShader.get());
    program.addShader(fragmentShader.get());

    if(!program.link(EShMessages::EShMsgDefault)) {
        return Core::Status::Error("Failed to link");
    }

    if(!program.buildReflection()) {
        return Core::Status::Error("Failed to build reflection data");
    }

    ASSIGN_OR_RETURN(std::string semanticsDefinitionsText, Core::IO::ReadTextFile(inputFiles.semanticsFile));
    auto semanticsDefinitions = nlohmann::json::parse(semanticsDefinitionsText);

    // We don't really care about outputs in the shader
    int pipeOutputCount = program.getNumPipeOutputs();
    Core::Log::Info(ShaderCompiler, "Pipe Outputs ({}):", pipeOutputCount);
    for(int i = 0; i < pipeOutputCount; i++) {
        const glslang::TObjectReflection& pipeOutput = program.getPipeOutput(i);
        Core::Log::Info(ShaderCompiler, "\t{}: {}", pipeOutput.name, pipeOutput.getType()->getCompleteString());
    }


    int bufferCount = program.getNumBufferVariables();
    if(bufferCount != 0) {
        Core::Log::Error(ShaderCompiler, "Buffers ({}):", bufferCount);
        for(int i = 0; i < bufferCount; i++) {
            const glslang::TObjectReflection& buffer = program.getBufferVariable(i);
            Core::Log::Error(ShaderCompiler, "\t{}: {}", buffer.name, buffer.getType()->getCompleteString());
        }
        return Core::Status::Error("Buffer variables are not currently supported");
    }

    int bufferBlockCount = program.getNumBufferBlocks();
    if(bufferBlockCount != 0) {
        Core::Log::Info(ShaderCompiler, "Buffer Blocks ({}):", bufferBlockCount);
        for(int i = 0; i < bufferBlockCount; i++) {
            const glslang::TObjectReflection& bufferBlock = program.getBufferBlock(i);
            Core::Log::Info(ShaderCompiler, "\t{}: {}", bufferBlock.name, bufferBlock.getType()->getCompleteString());
        }
        return Core::Status::Error("Buffer blocks are not currently supported");
    }

    Assets::Shader shader;
    shader.stageSources.emplace(
          Assets::PipelineStage::VERTEX,
          Assets::ShaderSource{.spirv = dumpShader(program, EShLanguage::EShLangVertex), .entryPoint = "main"});
    shader.stageSources.emplace(
          Assets::PipelineStage::FRAGMENT,
          Assets::ShaderSource{.spirv = dumpShader(program, EShLanguage::EShLangFragment), .entryPoint = "main"});

    int pipeInputCount = program.getNumPipeInputs();
    Core::Log::Info(ShaderCompiler, "Pipe Inputs ({}):", pipeInputCount);

    uint16_t nextInstanceAttributeByteOffset = 0;
    for(int i = 0; i < pipeInputCount; i++) {
        const glslang::TObjectReflection& pipeInput = program.getPipeInput(i);
        Core::Log::Info(ShaderCompiler, "\t{}: {}", pipeInput.name, pipeInput.getType()->getCompleteString());

        ASSIGN_OR_RETURN(Assets::VertexUsageName usage, GetVertexUsage(pipeInput.name, semanticsDefinitions));
        ASSIGN_OR_RETURN(Assets::VertexInputRateType rate, GetVertexRate(pipeInput.name, semanticsDefinitions));
        ASSIGN_OR_RETURN(Assets::VertexInput input, ToVertexInput(*pipeInput.getType(), usage, rate));
        shader.vertexInputs.emplace(pipeInput.name, input);

        if(rate == Assets::VertexInputRate::PER_INSTANCE) {
            shader.instanceFormat.properties.emplace(
                  usage,
                  Assets::BufferProperty{.property   = Assets::GLSL_ATTRIBUTE_TEMPLATE,
                                         .byteOffset = nextInstanceAttributeByteOffset,
                                         .count      = static_cast<uint8_t>(input.locationCount)});
            nextInstanceAttributeByteOffset += input.locationCount * Assets::GLSL_ATTRIBUTE_TEMPLATE.byteCount();
        }
    }

    Core::HashMap<std::string, Core::HashMap<std::string, uint8_t>> bufferOffsets;

    int uniformCount = program.getNumUniformVariables();
    Core::Log::Info(ShaderCompiler, "Uniforms ({}):", uniformCount);
    for(int i = 0; i < uniformCount; i++) {
        const glslang::TObjectReflection& uniform = program.getUniform(i);
        Core::Log::Info(ShaderCompiler, "\t{}: {}", uniform.name, uniform.getType()->getCompleteString());

        // An non -1 offset means its part of a block, which we will handle with the
        // uniform block reflection, but we need to save the offsets because they aren't stored in the type in the block
        // definition
        if(uniform.offset != -1) {
            Core::Array<std::string_view> nameSplit = Core::Algorithms::String::Split(uniform.name, '.');

            std::string bufferName(nameSplit[0]);
            std::string fieldName(nameSplit[1]);

            bufferOffsets[bufferName][fieldName] = uniform.offset;
            continue;
        }


        ASSIGN_OR_RETURN(Assets::ShaderUniform input, ToShaderUniform(uniform, bufferOffsets));
        shader.uniforms.emplace(uniform.name, input);
    }

    int uniformBlockCount = program.getNumUniformBlocks();
    Core::Log::Info(ShaderCompiler, "Uniform Blocks ({}):", uniformBlockCount);
    for(int i = 0; i < uniformBlockCount; i++) {
        const glslang::TObjectReflection& uniformBlock = program.getUniformBlock(i);
        Core::Log::Info(ShaderCompiler, "\t{}: {}", uniformBlock.name, uniformBlock.getType()->getCompleteString());

        ASSIGN_OR_RETURN(Assets::ShaderUniform input, ToShaderUniform(uniformBlock, bufferOffsets));
        shader.uniforms.emplace(uniformBlock.name, input);
    }

    ASSIGN_OR_RETURN(Core::IO::OutputStream outputStream, Core::IO::OpenFileForWrite(outputFile));
    outputStream.write(shader);

    glslang::FinalizeProcess();

    return Core::Status::Ok();
}

int main(int argc, char** argv) {
    Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Info);

    CLI::App app("Bengine Shader Compiler");

    SourceFiles sources;

    app.add_option("--vertex-source", sources.vertexFile, "Vertex stage source file to compile")
          ->required()
          ->check(CLI::ExistingFile);

    app.add_option("--fragment-source", sources.fragmentFile, "Fragment stage source file to compile")
          ->required()
          ->check(CLI::ExistingFile);

    app.add_option("--semantics-file", sources.semanticsFile, "Json file defining semantics for shader inputs")
          ->required()
          ->check(CLI::ExistingFile);

    app.add_option("--include-directories", sources.includeDirectories)->check(CLI::ExistingDirectory);


    std::filesystem::path outputFile;
    app.add_option("--output", outputFile, "Path to write the compiled output to")->required();

    app.add_flag_callback(
          "--quiet",
          []() { Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Error); },
          "Only print error messages");

    CLI11_PARSE(app, argc, argv);

    Core::Status status = run(sources, outputFile);

    if(status.isError()) {
        Core::Log::Error(ShaderCompiler, "Error compiling shader: {}", status.message());
        return EXIT_FAILURE;
    } else {
        Core::Log::Info(ShaderCompiler, "Done");
        return EXIT_SUCCESS;
    }
}
