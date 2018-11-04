#include "Core/FileSystem/FileSystem.h"

#include "Core/FileSystem/BareFileSystemMount.h"

namespace {
Core::FileSystem::BareFileSystemMount DefaultMount("");

Core::FileSystem::FileSystemMount*
findMountPoint(const Core::FileSystem::Path& path,
               Core::HashMap<std::filesystem::path, Core::FileSystem::FileSystemMount*> mounts) {
    std::filesystem::path pathToCheck = path.path;

    //Check each level of the path for a mount
    while(pathToCheck.has_relative_path()) {
        if(mounts.count(pathToCheck) != 0) {
            return mounts[pathToCheck];
        }

        pathToCheck = pathToCheck.parent_path();
    }

    //Check if the root path is a mount
    if(mounts.count(pathToCheck) != 0) {
        return mounts[pathToCheck];
    }

    return &DefaultMount;
}
}    // namespace

namespace Core::FileSystem {

FileSystem DefaultFileSystem;

std::optional<std::string> ReadTextFile(const Path& file) {
    return DefaultFileSystem.readTextFile(file);
}

std::optional<Core::Array<std::byte>> ReadBinaryFile(const Path& file) {
    return DefaultFileSystem.readBinaryFile(file);
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

    return findMountPoint(path, mounts)->translatePath(path);
}

std::optional<std::string> FileSystem::readTextFile(const Path& file) const {
    if (file.type == PathType::Explicit) {
        return DefaultMount.readTextFile(file);
    } else {
        return findMountPoint(file, mounts)->readTextFile(file);
    }
}

std::optional<Core::Array<std::byte>> FileSystem::readBinaryFile(const Path& file) const {
    if(file.type == PathType::Explicit) {
        return DefaultMount.readBinaryFile(file);
    } else {
        return findMountPoint(file, mounts)->readBinaryFile(file);
    }
}

void FileSystem::watchForChanges(const Path& file, const std::function<bool()>& observer) const {
    if(file.type == PathType::Explicit) {
        DefaultMount.watchForChanges(file, observer);
    } else {
        findMountPoint(file, mounts)->watchForChanges(file, observer);
    }
}
void FileSystem::updateWatchers() const {
    DefaultMount.updateWatchers();
    for(auto& mount : mounts) {
        mount.second->updateWatchers();
    }
}
}    // namespace Core::FileSystem