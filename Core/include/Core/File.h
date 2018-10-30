#pragma once

#include <filesystem>
#include <fstream>
#include <optional>

#include "Core/DllExport.h"
#include "Core/Containers/Array.h"

namespace Core::File {
CORE_API std::optional<std::string> ReadTextFile(const std::filesystem::path& file);
CORE_API std::optional<Core::Array<std::byte>> ReadBinaryFile(const std::filesystem::path& file);

class CORE_API Stream {
private:
    std::ifstream stream;

public:
    Stream(const std::filesystem::path& file);

    template <typename T>
    T read() {
        T value;
        stream.read(reinterpret_cast<std::byte*>(&value), sizeof(T));
        return value;
    }

    template <typename T>
    Core::Array<T> read(size_t count) {
        Core::Array<T> values(count);
        stream.read(reinterpret_cast<std::byte*>(values.data()), count * sizeof(T));
        return values;
    }
};
}    // namespace Core::File