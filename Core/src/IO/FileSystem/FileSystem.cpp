#include "Core/IO/FileSystem/FileSystem.h"

#include "Core/IO/FileSystem/BareFileSystemMount.h"

namespace {
Core::IO::BareFileSystemMount DefaultMount("");

const Core::IO::FileSystemMount&
findMountPoint(const Core::IO::Path& path,
               const Core::HashMap<std::filesystem::path, Core::IO::FileSystemMount*>& mounts) {
    std::filesystem::path pathToCheck = path.path.lexically_normal();

    // Check each level of the path for a mount
    while(pathToCheck.has_relative_path()) {
        if(mounts.count(pathToCheck) != 0) {
            return *mounts.at(pathToCheck);
        }

        pathToCheck = pathToCheck.parent_path();
    }

    // Check if the root path is a mount
    if(mounts.count(pathToCheck) != 0) {
        return *mounts.at(pathToCheck);
    }

    return DefaultMount;
}
}    // namespace

namespace Core::IO {

FileSystem DefaultFileSystem = FileSystem();

Core::StatusOr<InputStream> OpenFileForRead(const Path& file) {
    return DefaultFileSystem.openFileForRead(file);
}

Core::StatusOr<std::string> ReadTextFile(const Path& file) {
    return DefaultFileSystem.readTextFile(file);
}

Core::StatusOr<Core::Array<std::byte>> ReadBinaryFile(const Path& file) {
    return DefaultFileSystem.readBinaryFile(file);
}

Core::StatusOr<OutputStream> OpenFileForWrite(const Path& file) {
    return DefaultFileSystem.openFileForWrite(file);
}

void WriteBinaryFile(const Path& file, const Core::Array<std::byte>& data) {
    DefaultFileSystem.writeBinaryFile(file, data);
}

void WatchForChanges(const Path& file, const std::function<bool()>& observer) {
    DefaultFileSystem.watchForChanges(file, observer);
}


void FileSystem::addMount(FileSystemMount* mount) {
    mounts.emplace(mount->mountPath, mount);
}

std::filesystem::path FileSystem::translatePath(const Path& path) const {
    if(path.type == PathType::Explicit) {
        return path.path;
    }

    return findMountPoint(path, mounts).translatePath(path);
}

Core::StatusOr<InputStream> FileSystem::openFileForRead(const Path& file) const {
    if(file.type == PathType::Explicit) {
        return DefaultMount.openFileForRead(file);
    } else {
        return findMountPoint(file, mounts).openFileForRead(file);
    }
}

Core::StatusOr<std::string> FileSystem::readTextFile(const Path& file) const {
    if(file.type == PathType::Explicit) {
        return DefaultMount.readTextFile(file);
    } else {
        return findMountPoint(file, mounts).readTextFile(file);
    }
}

Core::StatusOr<Core::Array<std::byte>> FileSystem::readBinaryFile(const Path& file) const {
    if(file.type == PathType::Explicit) {
        return DefaultMount.readBinaryFile(file);
    } else {
        return findMountPoint(file, mounts).readBinaryFile(file);
    }
}

Core::StatusOr<OutputStream> FileSystem::openFileForWrite(const Path& file) const {
    if(file.type == PathType::Explicit) {
        return DefaultMount.openFileForWrite(file);
    } else {
        return findMountPoint(file, mounts).openFileForWrite(file);
    }
}

void FileSystem::writeBinaryFile(const Path& file, const Core::Array<std::byte>& data) const {
    if(file.type == PathType::Explicit) {
        DefaultMount.writeBinaryFile(file, data);
    } else {
        findMountPoint(file, mounts).writeBinaryFile(file, data);
    }
}

void FileSystem::watchForChanges(const Path& file, const std::function<bool()>& observer) const {
    if(file.type == PathType::Explicit) {
        DefaultMount.watchForChanges(file, observer);
    } else {
        findMountPoint(file, mounts).watchForChanges(file, observer);
    }
}
void FileSystem::updateWatchers() const {
    DefaultMount.updateWatchers();
    for(auto& mount : mounts) {
        mount.second->updateWatchers();
    }
}
}    // namespace Core::IO