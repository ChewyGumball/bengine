#pragma once

#include <filesystem>

#include "Core/DllExport.h"

namespace std {
template <>
struct hash<filesystem::path> {
    size_t operator()(const filesystem::path& s) const noexcept {
        return hash_value(s);
    }
};
}    // namespace std


namespace Core::IO {

enum class PathType { Mappable, Explicit };
struct CORE_API Path {
    std::filesystem::path path;
    PathType type;

    Path(const char* path, PathType type = PathType::Mappable);
    Path(const std::string& path, PathType type = PathType::Mappable);
    Path(const std::filesystem::path& path, PathType type = PathType::Mappable);
};

}    // namespace Core::FileSystem