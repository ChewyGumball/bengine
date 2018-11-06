#pragma once

#include <filesystem>
#include <functional>
#include <optional>

#include "Core/DllExport.h"

#include "Core/Containers/Array.h"
#include "Core/FileSystem/Path.h"
#include "Core/FileSystem/InputStream.h"

namespace Core::FileSystem {
struct CORE_API FileSystemMount {
    std::filesystem::path mountPath;

    FileSystemMount(const std::filesystem::path& mount);

    virtual std::filesystem::path translatePath(const Path& path) const = 0;

    virtual std::optional<InputStream> openFile(const Path& file) const                  = 0;
    virtual std::optional<std::string> readTextFile(const Path& file) const              = 0;
    virtual std::optional<Core::Array<std::byte>> readBinaryFile(const Path& file) const = 0;

    virtual void watchForChanges(const Path& file, const std::function<bool()>& observer) const = 0;
    virtual void updateWatchers() const                                                         = 0;
};
}    // namespace Core::FileSystem