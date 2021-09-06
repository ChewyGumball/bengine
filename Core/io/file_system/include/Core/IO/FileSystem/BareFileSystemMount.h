#pragma once

#include "Core/IO/FileSystem/FileSystemMount.h"

namespace Core::IO {
struct BareFileSystemMount : FileSystemMount {
    BareFileSystemMount(const std::filesystem::path& mount);

    std::filesystem::path translatePath(const Path& path) const override;

    Core::StatusOr<InputStream> openFileForRead(const Path& file) const override;
    Core::StatusOr<std::string> readTextFile(const Path& file) const override;
    Core::StatusOr<Core::Array<std::byte>> readBinaryFile(const Path& file) const override;

    Core::StatusOr<OutputStream> openFileForWrite(const Path& file) const override;
    void writeBinaryFile(const Path& file, std::span<const std::byte> data) const override;

    void watchForChanges(const Path& file, const std::function<Core::Status()>& observer) const override;
    Core::Status updateWatchers() const override;
};
}    // namespace Core::IO