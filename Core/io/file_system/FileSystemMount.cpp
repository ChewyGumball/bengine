#include "Core/IO/FileSystem/FileSystemMount.h"

namespace {
std::filesystem::path fixupPath(const std::filesystem::path& path) {
    if(!path.has_filename() && path.has_parent_path()) {
        return path.parent_path().lexically_normal();
    } else {
        return path.lexically_normal();
    }
}
}    // namespace

namespace Core::IO {

FileSystemMount::FileSystemMount(const std::filesystem::path& mount) : mountPath(fixupPath(mount)) {}
}    // namespace Core::FileSystem