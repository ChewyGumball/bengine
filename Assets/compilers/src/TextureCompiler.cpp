
#include <Assets/Compilers/TextureCompiler.h>

#include <Assets/Textures/Texture.h>
#include <Core/Containers/Array.h>
#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/Logging/Logger.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {
Core::LogCategory Compiler("Texture Compiler");
}

namespace Assets::TextureCompiler {
Core::Status Compile(const std::filesystem::path& inputFile, const std::filesystem::path& outputFile) {
    ASSIGN_OR_RETURN(Core::Array<std::byte> imageData, Core::IO::ReadBinaryFile(inputFile));

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<unsigned char*>(imageData.rawData()),
                                            imageData.count(),
                                            &texWidth,
                                            &texHeight,
                                            &texChannels,
                                            STBI_rgb_alpha);
    if(pixels == nullptr) {
        return Core::Status::Error("Failed to load texture '{}'", inputFile);
    }

    std::span<std::byte> data(reinterpret_cast<std::byte*>(pixels), texWidth * texHeight * 4);

    Texture texture{
          .height = static_cast<uint32_t>(texHeight),
          .width  = static_cast<uint32_t>(texWidth),
          .format = TextureFormat::RGBA_8_UINT,
          .data   = Core::Array<std::byte>(data),
    };

    stbi_image_free(pixels);

    ASSIGN_OR_RETURN(Core::IO::OutputStream outputStream, Core::IO::OpenFileForWrite(outputFile));
    outputStream.write(texture);

    Core::Log::Debug(Compiler, "Compiled {} to {}", inputFile.string(), outputFile.string());
    Core::Log::Debug(Compiler, "Width: {}, Height: {}", texWidth, texHeight);

    return Core::Status::Ok();
}
}    // namespace Assets::TextureCompiler
