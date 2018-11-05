#include "Core/FileSystem/FileSystemMount.h"

namespace Core::FileSystem {

FileSystemMount::FileSystemMount(const std::filesystem::path& mount) : mountPath(mount.lexically_normal()) {
    if (!mountPath.has_filename()) {
        mountPath = mountPath.parent_path();
    }
}
}    // namespace Core::FileSystem