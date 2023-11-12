
#include <cstdlib>
#include <filesystem>

#include "core/containers/Array.h"
#include "core/containers/HashMap.h"

#include "core/io/file_system/FileSystem.h"
#include "core/io/serialization/ArrayBuffer.h"
#include "core/io/serialization/OutputStream.h"
#include "core/logging/Logger.h"


#include "assets/importers/obj/OBJImporter.h"

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
    Core::Log::Debug(Compiler, "Parts:");
    for(auto& part : mesh.meshParts) {
        Core::Log::Debug(Compiler, "\t{}: {} offset, {} indices", part.name, part.indices.start, part.indices.count);
    }

    return Core::Status::Ok();
}
}    // namespace Assets::ModelCompiler
