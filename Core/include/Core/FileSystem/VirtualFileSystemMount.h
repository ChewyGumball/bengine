#pragma once

#include "Core/FileSystem/BareFileSystemMount.h"

namespace Core::FileSystem {
struct CORE_API VirtualFileSystemMount : public BareFileSystemMount {
    std::filesystem::path rootPath;

    VirtualFileSystemMount(const std::filesystem::path& mount, const std::filesystem::path& root);
    virtual std::filesystem::path translatePath(const Path& path) const;

    virtual std::optional<std::string> readTextFile(const Path& file) const;
    virtual std::optional<Core::Array<std::byte>> readBinaryFile(const Path& file) const;

    virtual void watchForChanges(const Path& file, const std::function<bool()>& observer) const;
    virtual void updateWatchers() const;
};
}    // namespace Core::FileSystem