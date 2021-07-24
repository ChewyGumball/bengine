
#include <cstdlib>
#include <filesystem>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/OutputStream.h>
#include <Core/Logging/Logger.h>

#include <Assets/Importers/OBJImporter.h>

Core::LogCategory ModelCompiler("Model Compiler");

Core::Status run(std::filesystem::path& file) {
    std::filesystem::path outputFile = std::filesystem::path(file).replace_extension(".mesh");
    ASSIGN_OR_RETURN(Core::IO::OutputStream outputStream, Core::IO::OpenFileForWrite(outputFile));

    Assets::Mesh mesh;
    if(file.extension() == ".obj") {
        mesh = Assets::OBJ::Import(file);
    } else {
        return Core::Status::Error("Unknown file extension: '{}'", file.extension().string());
    }

    mesh.deduplicateVertices();
    mesh.optimizeVertexOrder();

    outputStream.write(mesh);

    Core::Log::Info(ModelCompiler, "Compiled {} to {}", file.string(), outputFile.string());
    Core::Log::Info(ModelCompiler,
                    "Vertices: {} ({} bytes)",
                    mesh.vertexData.count() / mesh.vertexFormat.byteCount(),
                    mesh.vertexData.count());
    Core::Log::Info(
          ModelCompiler, "Indices: {} ({} bytes)", mesh.indexData.count(), mesh.indexData.count() * sizeof(uint32_t));
    Core::Log::Info(ModelCompiler, "Format:");
    for(auto& [usage, property] : mesh.vertexFormat.properties) {
        Core::Log::Info(ModelCompiler,
                        "\t{}: {} {} elements at offset {}",
                        Assets::VertexUsage::AsString(usage),
                        property.property.elementCount,
                        Assets::PropertyType::AsString(property.property.type),
                        property.byteOffset);
    }

    return Core::Status::Ok();
}


int main(int argc, char** argv) {
    Core::Array<std::string> args(argc);
    for(int i = 0; i < argc; i++) {
        args.emplace(argv[i]);
    }

    std::filesystem::path file = args[1];
    Core::Status status        = run(file);

    if(status.isError()) {
        Core::Log::Error(ModelCompiler, "Error compiling model: {}", status.message());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
