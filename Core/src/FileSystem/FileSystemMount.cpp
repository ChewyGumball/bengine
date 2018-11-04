#include "Core/FileSystem/FileSystemMount.h"

namespace Core::FileSystem {

FileSystemMount::FileSystemMount(const std::filesystem::path& mount) : mountPath(mount) {}
}    // namespace Core::FileSystem