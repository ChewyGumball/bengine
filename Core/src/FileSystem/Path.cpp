#include "Core/FileSystem/Path.h"

namespace Core::FileSystem {
Path::Path(const char* path, PathType type) : path(path), type(type) {}
Path::Path(const std::string& path, PathType type) : path(path), type(type) {}
Path::Path(const std::filesystem::path& path, PathType type) : path(path), type(type) {}
}    // namespace Core::FileSystem