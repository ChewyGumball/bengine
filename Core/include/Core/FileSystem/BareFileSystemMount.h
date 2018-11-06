#pragma once

#include "Core/DllExport.h"

#include "Core/FileSystem/FileSystemMount.h"

namespace Core::FileSystem {
struct CORE_API BareFileSystemMount : FileSystemMount {

    BareFileSystemMount(const std::filesystem::path& mount);

    virtual std::filesystem::path translatePath(const Path& path) const;

    virtual std::optional<InputStream> openFile(const Path& file) const;
    virtual std::optional<std::string> readTextFile(const Path& file) const;
    virtual std::optional<Core::Array<std::byte>> readBinaryFile(const Path& file) const;

    virtual void watchForChanges(const Path& file, const std::function<bool()>& observer) const;
    virtual void updateWatchers() const;
};
}    // namespace Core::FileSystem