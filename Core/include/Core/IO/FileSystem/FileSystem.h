#pragma once

#include <filesystem>
#include <functional>

#include <Core/IO/FileSystem/FileSystemMount.h>
#include <Core/IO/FileSystem/Path.h>
#include <Core/IO/InputStream.h>
#include <Core/IO/OutputStream.h>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

namespace Core::IO {
struct FileSystem {
private:
    Core::HashMap<std::filesystem::path, FileSystemMount*> mounts;

public:
    void addMount(FileSystemMount* mount);

    std::filesystem::path translatePath(const Path& path) const;

    Core::StatusOr<InputStream> openFileForRead(const Path& file) const;
    Core::StatusOr<std::string> readTextFile(const Path& file) const;
    Core::StatusOr<Core::Array<std::byte>> readBinaryFile(const Path& file) const;

    Core::StatusOr<OutputStream> openFileForWrite(const Path& file) const;
    void writeBinaryFile(const Path& file, const Core::Array<std::byte>& data) const;

    void watchForChanges(const Path& file, const std::function<bool()>& observer) const;
    void updateWatchers() const;
};

extern FileSystem DefaultFileSystem;

Core::StatusOr<InputStream> OpenFileForRead(const Path& file);
Core::StatusOr<std::string> ReadTextFile(const Path& file);
Core::StatusOr<Core::Array<std::byte>> ReadBinaryFile(const Path& file);

Core::StatusOr<OutputStream> OpenFileForWrite(const Path& file);
void WriteBinaryFile(const Path& file, const Core::Array<std::byte>& data);

void WatchForChanges(const Path& file, const std::function<bool()>& observer);
}    // namespace Core::IO