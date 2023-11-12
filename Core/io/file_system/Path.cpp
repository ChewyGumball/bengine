#include "core/io/file_system/Path.h"

namespace Core::IO {
Path::Path(const char* path, PathType type) : path(path), type(type) {}
Path::Path(const std::string& path, PathType type) : path(path), type(type) {}
Path::Path(const std::filesystem::path& path, PathType type) : path(path), type(type) {}
}    // namespace Core::IO