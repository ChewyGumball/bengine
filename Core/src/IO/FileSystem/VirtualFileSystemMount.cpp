#include "Core/IO/FileSystem/VirtualFileSystemMount.h"

#include <assert.h>

namespace Core::IO {
VirtualFileSystemMount::VirtualFileSystemMount(const std::filesystem::path& mount, const std::filesystem::path& root)
  : BareFileSystemMount(mount), rootPath(root.lexically_normal()) {}

std::filesystem::path VirtualFileSystemMount::translatePath(const Path& path) const {
    return rootPath / path.path.lexically_relative(mountPath);
}

std::optional<InputStream> VirtualFileSystemMount::openFileForRead(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::openFileForRead(translatedPath);
}

std::optional<std::string> VirtualFileSystemMount::readTextFile(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::readTextFile(translatedPath);
}
std::optional<Core::Array<std::byte>> VirtualFileSystemMount::readBinaryFile(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::readBinaryFile(translatedPath);
}


std::optional<OutputStream> VirtualFileSystemMount::openFileForWrite(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::openFileForWrite(translatedPath);
}

void VirtualFileSystemMount::writeBinaryFile(const Path& file, const Core::Array<std::byte>& data) const {
    Path translatedPath(translatePath(file), file.type);
    BareFileSystemMount::writeBinaryFile(translatedPath, data);
}

void VirtualFileSystemMount::watchForChanges(const Path& file, const std::function<bool()>& observer) const {
    Path translatedPath(translatePath(file), file.type);
    BareFileSystemMount::watchForChanges(translatedPath, observer);
}

void VirtualFileSystemMount::updateWatchers() const {
    BareFileSystemMount::updateWatchers();
}
}    // namespace Core::IO