#pragma once

#include <Core/Algorithms/Hashing.h>

#include <filesystem>

namespace Core::IO {

enum class PathType { Mappable, Explicit };
struct Path {
    std::filesystem::path path;
    PathType type;

    Path(const char* path, PathType type = PathType::Mappable);
    Path(const std::string& path, PathType type = PathType::Mappable);
    Path(const std::filesystem::path& path, PathType type = PathType::Mappable);
};
}    // namespace Core::IO


namespace std {
template <>
struct hash<Core::IO::Path> {
    size_t operator()(const Core::IO::Path& value) const noexcept {
        return Core::Algorithms::CombineHashes(value.path, value.type);
    }
};
}    // namespace std
