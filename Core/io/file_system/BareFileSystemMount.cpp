#include "core/io/file_system/BareFileSystemMount.h"

#include "core/assert/Assert.h"
#include "core/containers/HashMap.h"
#include "core/containers/HashSet.h"

#include <FileWatcher/FileWatcher.h>

#include <fstream>

namespace {
std::unique_ptr<FW::FileWatcher> fileWatcher = std::make_unique<FW::FileWatcher>();

class UpdateListener : public FW::FileWatchListener {
private:
    Core::HashMap<std::filesystem::path, Core::Array<std::function<Core::Status()>>> watchers;
    Core::HashSet<std::filesystem::path> watchedDirectories;

    Core::HashSet<std::function<Core::Status()>*> deduplicatedObservers;

public:
    void handleFileAction(FW::WatchID watchID, const FW::String& dir, const FW::String& file, FW::Action action) {
        std::filesystem::path directory = dir;
        std::filesystem::path filename  = file;
        std::filesystem::path fullPath  = directory / filename;
        if(action == FW::Action::Modified) {
            if(watchers.count(fullPath) > 0) {
                for(auto& observer : watchers[fullPath]) {
                    deduplicatedObservers.insert(&observer);
                }
            }
        }
    }

    Core::Status postProcess() {
        for(auto observer : deduplicatedObservers) {
            RETURN_IF_ERROR((*observer)());
        }

        deduplicatedObservers.clear();
        return Core::Status::Ok();
    }

    void add(const std::filesystem::path& filename, const std::function<Core::Status()>& observer) {
        watchers[filename].emplace(observer);
        std::filesystem::path directory = filename.parent_path();
        if(watchedDirectories.count(directory) == 0) {
            fileWatcher->addWatch(directory.string(), this);
            watchedDirectories.insert(directory);
        }
    }
} FileUpdateListener;
}    // namespace

namespace Core::IO {

BareFileSystemMount::BareFileSystemMount(const std::filesystem::path& mount) : FileSystemMount(mount) {}

std::filesystem::path BareFileSystemMount::translatePath(const Path& path) const {
    return path.path;
}

Core::StatusOr<InputStream> BareFileSystemMount::openFileForRead(const Path& file) const {
    std::unique_ptr<class std::basic_istream<std::byte>> reader =
          std::make_unique<std::basic_ifstream<std::byte>>(file.path, std::ios::in | std::ios::binary);

    if(*reader) {
        return Core::IO::InputStream(std::move(reader));
    } else {
        return Core::Status::Error("Unable to open file to read at path: {}", file.path.string());
    }
}

Core::StatusOr<std::string> BareFileSystemMount::readTextFile(const Path& file) const {
    std::ifstream reader(file.path, std::ios::in);
    if(!reader) {
        return Core::Status::Error("Unable to open file to read at path: {}", file.path.string());
    }

    uint64_t fileSize = std::filesystem::file_size(file.path);
    std::string contents(fileSize, '\0');
    reader.read(contents.data(), fileSize);
    return contents;
}

Core::StatusOr<Core::Array<std::byte>> BareFileSystemMount::readBinaryFile(const Path& file) const {
    std::basic_ifstream<std::byte> reader(file.path, std::ios::in | std::ios::binary);
    if(!reader) {
        return Core::Status::Error("Unable to open file to read at path: {}", file.path.string());
    }

    uint64_t fileSize = std::filesystem::file_size(file.path);

    Core::Array<std::byte> contents;
    Core::Span<std::byte> data = contents.insertUninitialized(fileSize);

    reader.read(data.rawData(), data.count());
    return contents;
}

Core::StatusOr<OutputStream> BareFileSystemMount::openFileForWrite(const Path& file) const {
    std::unique_ptr<std::basic_ostream<std::byte>> writer =
          std::make_unique<std::basic_ofstream<std::byte>>(file.path, std::ios::out | std::ios::binary);

    if(*writer) {
        return OutputStream(std::move(writer));
    } else {
        return Core::Status::Error("Unable to open file to write at path: {}", file.path.string());
    }
}

void BareFileSystemMount::writeBinaryFile(const Path& file, std::span<const std::byte> data) const {
    std::basic_ofstream<std::byte> writer(file.path, std::ios::out | std::ios::binary);
    ASSERT(writer);
    writer.write(data.data(), data.size());
}

void BareFileSystemMount::watchForChanges(const Path& file, const std::function<Core::Status()>& observer) const {
    FileUpdateListener.add(file.path, observer);
}
Core::Status BareFileSystemMount::updateWatchers() const {
    fileWatcher->update();
    return FileUpdateListener.postProcess();
}
}    // namespace Core::IO