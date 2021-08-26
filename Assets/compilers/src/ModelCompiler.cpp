
#include <cstdlib>
#include <filesystem>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/OutputStream.h>
#include <Core/Logging/Logger.h>

#include <Assets/Importers/OBJImporter.h>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>


Core::LogCategory ModelCompiler("Model Compiler");

Core::Status run(const std::filesystem::path& inputFile, const std::filesystem::path& outputFile) {
    ASSIGN_OR_RETURN(Core::IO::OutputStream outputStream, Core::IO::OpenFileForWrite(outputFile));

    Assets::Mesh mesh;
    if(inputFile.extension() == ".obj") {
        mesh = Assets::OBJ::Import(inputFile);
    } else {
        return Core::Status::Error("Unknown file extension: '{}'", inputFile.extension().string());
    }

    mesh.deduplicateVertices();
    mesh.optimizeVertexOrder();

    outputStream.write(mesh);

    Core::Log::Debug(ModelCompiler, "Compiled {} to {}", inputFile.string(), outputFile.string());
    Core::Log::Debug(ModelCompiler,
                     "Vertices: {} ({} bytes)",
                     mesh.vertexData.count() / mesh.vertexFormat.byteCount(),
                     mesh.vertexData.count());
    Core::Log::Debug(
          ModelCompiler, "Indices: {} ({} bytes)", mesh.indexData.count(), mesh.indexData.count() * sizeof(uint32_t));
    Core::Log::Debug(ModelCompiler, "Format:");
    for(auto& [usage, property] : mesh.vertexFormat.properties) {
        Core::Log::Debug(ModelCompiler,
                         "\t{}: {} {} elements at offset {}",
                         Assets::VertexUsage::AsString(usage),
                         property.property.elementCount,
                         Assets::PropertyType::AsString(property.property.type),
                         property.byteOffset);
    }

    return Core::Status::Ok();
}


int main(int argc, char** argv) {
    Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Info);

    CLI::App app("Bengine Model Compiler");

    std::filesystem::path inputFile;
    app.add_option("--input", inputFile, "Input file to compile")->required()->check(CLI::ExistingFile);

    std::filesystem::path outputFile;
    app.add_option("--output", outputFile, "Path to write the compiled output to")->required();

    app.add_flag_callback(
          "--quiet",
          []() { Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Error); },
          "Only print error messages");

    CLI11_PARSE(app, argc, argv);

    Core::Status status = run(inputFile, outputFile);

    if(status.isError()) {
        Core::Log::Error(ModelCompiler, "Error compiling model: {}", status.message());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
