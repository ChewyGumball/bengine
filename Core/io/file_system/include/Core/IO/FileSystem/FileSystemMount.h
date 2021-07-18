#pragma once

#include "Core/IO/FileSystem/Path.h"

#include <Core/Containers/Array.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>
#include <Core/Status/StatusOr.h>

#include <filesystem>
#include <functional>

namespace Core::IO {
struct FileSystemMount {
    std::filesystem::path mountPath;

    FileSystemMount(const std::filesystem::path& mount);

    virtual std::filesystem::path translatePath(const Path& path) const = 0;

    virtual Core::StatusOr<InputStream> openFileForRead(const Path& file) const           = 0;
    virtual Core::StatusOr<std::string> readTextFile(const Path& file) const              = 0;
    virtual Core::StatusOr<Core::Array<std::byte>> readBinaryFile(const Path& file) const = 0;

    virtual Core::StatusOr<OutputStream> openFileForWrite(const Path& file) const         = 0;
    virtual void writeBinaryFile(const Path& file, std::span<const std::byte> data) const = 0;

    virtual void watchForChanges(const Path& file, const std::function<bool()>& observer) const = 0;
    virtual void updateWatchers() const                                                         = 0;
};
}    // namespace Core::IO