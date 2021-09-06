
#include <cstdlib>
#include <filesystem>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/OutputStream.h>
#include <Core/Logging/Logger.h>

#include <Assets/Importers/OBJImporter.h>

namespace {
Core::LogCategory Compiler("Model Compiler");
}

namespace Assets::ModelCompiler {

Core::Status Compile(const std::filesystem::path& inputFile, const std::filesystem::path& outputFile) {
    Assets::Mesh mesh;
    if(inputFile.extension() == ".obj") {
        mesh = Assets::OBJ::Import(inputFile);
    } else {
        return Core::Status::Error("Unknown file extension: '{}'", inputFile.extension().string());
    }

    mesh.deduplicateVertices();
    mesh.optimizeVertexOrder();

    ASSIGN_OR_RETURN(Core::IO::OutputStream outputStream, Core::IO::OpenFileForWrite(outputFile));
    outputStream.write(mesh);

    Core::Log::Debug(Compiler, "Compiled {} to {}", inputFile.string(), outputFile.string());
    Core::Log::Debug(Compiler,
                     "Vertices: {} ({} bytes)",
                     mesh.vertexData.count() / mesh.vertexFormat.byteCount(),
                     mesh.vertexData.count());
    Core::Log::Debug(
          Compiler, "Indices: {} ({} bytes)", mesh.indexData.count(), mesh.indexData.count() * sizeof(uint32_t));
    Core::Log::Debug(Compiler, "Format:");
    for(auto& [usage, property] : mesh.vertexFormat.properties) {
        Core::Log::Debug(Compiler,
                         "\t{}: {} {} elements at offset {}",
                         Assets::VertexUsage::AsString(usage),
                         property.property.elementCount,
                         Assets::PropertyType::AsString(property.property.type),
                         property.byteOffset);
    }

    return Core::Status::Ok();
}
}    // namespace Assets::ModelCompiler
