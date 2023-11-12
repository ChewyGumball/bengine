#pragma once

#include "core/io/file_system/FileSystemMount.h"
#include "core/io/file_system/Path.h"

#include "core/containers/Array.h"
#include "core/containers/HashMap.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"


#include <filesystem>
#include <functional>
#include <span>

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
    void writeBinaryFile(const Path& file, std::span<const std::byte> data) const;

    void watchForChanges(const Path& file, const std::function<Core::Status()>& observer) const;
    Core::Status updateWatchers() const;
};

extern FileSystem DefaultFileSystem;

Core::StatusOr<InputStream> OpenFileForRead(const Path& file);
Core::StatusOr<std::string> ReadTextFile(const Path& file);
Core::StatusOr<Core::Array<std::byte>> ReadBinaryFile(const Path& file);

Core::StatusOr<OutputStream> OpenFileForWrite(const Path& file);
void WriteBinaryFile(const Path& file, const Core::Array<std::byte>& data);

void WatchForChanges(const Path& file, const std::function<Core::Status()>& observer);
}    // namespace Core::IO