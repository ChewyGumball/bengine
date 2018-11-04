#include "Core/FileSystem/VirtualFileSystemMount.h"

#include <assert.h>

namespace Core::FileSystem {
VirtualFileSystemMount::VirtualFileSystemMount(const std::filesystem::path& mount, const std::filesystem::path& root)
  : BareFileSystemMount(mount), rootPath(root) {}

std::filesystem::path VirtualFileSystemMount::translatePath(const Path& path) const {
    std::string originalPath = path.path.string();
    std::string mountPrefix  = mountPath.string();

    assert(std::string_view(originalPath).substr(0, mountPrefix.size()) == mountPrefix);

    return rootPath / originalPath.substr(mountPrefix.size());
}

std::optional<std::string> VirtualFileSystemMount::readTextFile(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::readTextFile(translatedPath);
}
std::optional<Core::Array<std::byte>> VirtualFileSystemMount::readBinaryFile(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::readBinaryFile(translatedPath);
}

void VirtualFileSystemMount::watchForChanges(const Path& file, const std::function<bool()>& observer) const {
    Path translatedPath(translatePath(file), file.type);
    BareFileSystemMount::watchForChanges(translatedPath, observer);
}

void VirtualFileSystemMount::updateWatchers() const {
    BareFileSystemMount::updateWatchers();
}
}    // namespace Core::FileSystem