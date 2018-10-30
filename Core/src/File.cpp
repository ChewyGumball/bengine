#include "Core/File.h"

#include <fstream>
#include <sstream>

namespace Core::File {
std::optional<std::string> ReadTextFile(const std::filesystem::path& file) {
    std::ifstream reader(file, std::ios::in);
    if(!reader) {
        return std::nullopt;
    }

    /*
    // New lines may be converted during reading, so the final size may not be the same as the file size. Using a string
    // stream is the best way to get a properly sized final string.
    std::ostringstream fileContents;
    fileContents << reader.rdbuf();
    reader.close();

    return fileContents.str();
    */
    
    uint64_t fileSize = std::filesystem::file_size(file);
    std::string contents(fileSize, '\0');
    reader.read(contents.data(), fileSize);
    return contents;
}

std::optional<Core::Array<std::byte>> ReadBinaryFile(const std::filesystem::path& file) {
    std::basic_ifstream<std::byte> reader(file, std::ios::in | std::ios::binary);
    if(!reader) {
        return std::nullopt;
    }

    uint64_t fileSize = std::filesystem::file_size(file);
    Core::Array<std::byte> contents(fileSize);
    reader.read(contents.data(), fileSize);
    return contents;
}

Stream::Stream(const std::filesystem::path& file) : stream(file, std::ios::in | std::ios::binary) {}
}    // namespace Core::File