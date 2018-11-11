#pragma once

#include <filesystem>
#include <functional>
#include <optional>

#include "Core/Containers/Array.h"
#include "Core/Containers/HashMap.h"
#include "Core/IO/InputStream.h"
#include "Core/IO/OutputStream.h"

#include "Core/DllExport.h"

#include "FileSystemMount.h"
#include "Path.h"

namespace Core::IO {
struct CORE_API FileSystem {
private:
    Core::HashMap<std::filesystem::path, FileSystemMount*> mounts;

public:
    void addMount(FileSystemMount* mount);

    std::filesystem::path translatePath(const Path& path) const;

    std::optional<InputStream> openFileForRead(const Path& file) const;
    std::optional<std::string> readTextFile(const Path& file) const;
    std::optional<Core::Array<std::byte>> readBinaryFile(const Path& file) const;

    std::optional<OutputStream> openFileForWrite(const Path& file) const;
    void writeBinaryFile(const Path& file, const Core::Array<std::byte>& data) const;

    void watchForChanges(const Path& file, const std::function<bool()>& observer) const;
    void updateWatchers() const;
};

extern CORE_API FileSystem DefaultFileSystem;

std::optional<InputStream> CORE_API OpenFileForRead(const Path& file);
std::optional<std::string> CORE_API ReadTextFile(const Path& file);
std::optional<Core::Array<std::byte>> CORE_API ReadBinaryFile(const Path& file);

std::optional<OutputStream> CORE_API OpenFileForWrite(const Path& file);
void CORE_API WriteBinaryFile(const Path& file, const Core::Array<std::byte>& data);

void CORE_API WatchForChanges(const Path& file, const std::function<bool()>& observer);
}    // namespace Core::FileSystem