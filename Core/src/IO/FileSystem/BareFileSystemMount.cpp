#include "Core/IO/FileSystem/BareFileSystemMount.h"

#include <assert.h>
#include <fstream>

#include <FileWatcher/FileWatcher.h>

#include "Core/Containers/HashMap.h"
#include "Core/Containers/HashSet.h"

namespace {
std::unique_ptr<FW::FileWatcher> fileWatcher = std::make_unique<FW::FileWatcher>();

class UpdateListener : public FW::FileWatchListener {
private:
    Core::HashMap<std::filesystem::path, Core::Array<std::function<bool()>>> watchers;
    Core::HashSet<std::filesystem::path> watchedDirectories;

    Core::HashSet<std::function<bool()>*> duplicateChanges;

public:
    void handleFileAction(FW::WatchID watchID, const std::filesystem::path& path, FW::Action action) {
        if(action == FW::Action::Modified) {
            if(watchers.count(path) > 0) {
                for(auto& observer : watchers[path]) {
                    duplicateChanges.insert(&observer);
                }
            }
        }
    }

    void postProcess() {
        for(auto it = duplicateChanges.begin(); it != duplicateChanges.end();) {
            std::function<bool()>* observer = *it;

            if((*observer)()) {
                it = duplicateChanges.erase(it);
            } else {
                ++it;
            }
        }
    }

    void add(const std::filesystem::path& filename, const std::function<bool()>& observer) {
        watchers[filename].emplace_back(observer);
        std::filesystem::path directory = filename.parent_path();
        if(watchedDirectories.count(directory) == 0) {
            fileWatcher->addWatch(directory, this);
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

std::optional<InputStream> BareFileSystemMount::openFileForRead(const Path& file) const {
    std::unique_ptr<class std::basic_istream<std::byte>> reader =
          std::make_unique<std::basic_ifstream<std::byte>>(file.path, std::ios::in);

    if(*reader) {
        return Core::IO::InputStream(std::move(reader));
    } else {
        return std::nullopt;
    }
}

std::optional<std::string> BareFileSystemMount::readTextFile(const Path& file) const {
    std::ifstream reader(file.path, std::ios::in);
    if(!reader) {
        return std::nullopt;
    }

    uint64_t fileSize = std::filesystem::file_size(file.path);
    std::string contents(fileSize, '\0');
    reader.read(contents.data(), fileSize);
    return contents;
}

std::optional<Core::Array<std::byte>> BareFileSystemMount::readBinaryFile(const Path& file) const {
    std::basic_ifstream<std::byte> reader(file.path, std::ios::in | std::ios::binary);
    if(!reader) {
        return std::nullopt;
    }

    uint64_t fileSize = std::filesystem::file_size(file.path);
    Core::Array<std::byte> contents(fileSize);
    reader.read(contents.data(), fileSize);
    return contents;
}

std::optional<OutputStream> BareFileSystemMount::openFileForWrite(const Path& file) const {
    std::unique_ptr<std::basic_ostream<std::byte>> writer =
          std::make_unique<std::basic_ofstream<std::byte>>(file.path, std::ios::out | std::ios::binary);

    if(*writer) {
        return OutputStream(std::move(writer));
    } else {
        return std::nullopt;
    }
}

void BareFileSystemMount::writeBinaryFile(const Path& file, const Core::Array<std::byte>& data) const {
    std::basic_ofstream<std::byte> writer(file.path, std::ios::out | std::ios::binary);
    assert(writer);
    writer.write(data.data(), data.size());
}

void BareFileSystemMount::watchForChanges(const Path& file, const std::function<bool()>& observer) const {
    FileUpdateListener.add(file.path, observer);
}
void BareFileSystemMount::updateWatchers() const {
    fileWatcher->update();
    FileUpdateListener.postProcess();
}
}    // namespace Core::IO