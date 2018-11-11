
#include <cstdlib>
#include <filesystem>

#include <Core/Containers/Array.h>
#include <Core/Logging/Logger.h>
#include <Core/IO/ArrayBuffer.h>
#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/OutputStream.h>

#include <Assets/Importers/OBJImporter.h>

Core::LogCategory ModelCompiler("Model Compiler");

int main(int argc, char** argv)
{
    Core::Array<std::string> args(argc);
    for (int i = 0; i < argc; i++) {
        args[i] = argv[i];
    }

    std::filesystem::path file = args[1];

    std::filesystem::path outputFile = file.replace_extension(".mesh");
    std::optional<Core::IO::OutputStream> outputStream = Core::IO::OpenFileForWrite(outputFile);
    if (!outputStream) {
        Core::Log::Error(ModelCompiler, "Could not open output file: '{}'", outputFile.string());
        return EXIT_FAILURE;
    }

    Assets::Mesh mesh;
    if (file.extension() == ".obj") {
        mesh = Assets::OBJ::Import(file);
    }
    else {
        Core::Log::Error(ModelCompiler, "Unknown file extension: '{}'", file.extension().string());
        return EXIT_FAILURE;
    }

    mesh.deduplicateVertices();
    mesh.optimizeVertexOrder();

    outputStream->write(mesh);

    return EXIT_SUCCESS;
}
