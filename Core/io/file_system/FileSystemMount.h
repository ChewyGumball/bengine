#pragma once

#include "core/io/file_system/Path.h"

#include "core/containers/Array.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"
#include "core/status/StatusOr.h"

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

    virtual void watchForChanges(const Path& file, const std::function<Core::Status()>& observer) const = 0;
    virtual Core::Status updateWatchers() const                                                         = 0;
};
}    // namespace Core::IO