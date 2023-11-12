#include "core/io/file_system/VirtualFileSystemMount.h"

namespace Core::IO {
VirtualFileSystemMount::VirtualFileSystemMount(const std::filesystem::path& mount, const std::filesystem::path& root)
  : BareFileSystemMount(mount), rootPath(root.lexically_normal()) {}

std::filesystem::path VirtualFileSystemMount::translatePath(const Path& path) const {
    return rootPath / path.path.lexically_relative(mountPath);
}

Core::StatusOr<InputStream> VirtualFileSystemMount::openFileForRead(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::openFileForRead(translatedPath);
}

Core::StatusOr<std::string> VirtualFileSystemMount::readTextFile(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::readTextFile(translatedPath);
}
Core::StatusOr<Core::Array<std::byte>> VirtualFileSystemMount::readBinaryFile(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::readBinaryFile(translatedPath);
}


Core::StatusOr<OutputStream> VirtualFileSystemMount::openFileForWrite(const Path& file) const {
    Path translatedPath(translatePath(file), file.type);
    return BareFileSystemMount::openFileForWrite(translatedPath);
}

void VirtualFileSystemMount::writeBinaryFile(const Path& file, std::span<const std::byte> data) const {
    Path translatedPath(translatePath(file), file.type);
    BareFileSystemMount::writeBinaryFile(translatedPath, data);
}

void VirtualFileSystemMount::watchForChanges(const Path& file, const std::function<Core::Status()>& observer) const {
    Path translatedPath(translatePath(file), file.type);
    BareFileSystemMount::watchForChanges(translatedPath, observer);
}

Core::Status VirtualFileSystemMount::updateWatchers() const {
    return BareFileSystemMount::updateWatchers();
}
}    // namespace Core::IO