#pragma once

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>
#include <Core/Containers/OpaqueID.h>
#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/Status/Status.h>


#include <filesystem>

namespace Assets {
template <typename ASSET_TYPE>
class AssetCatalog {
private:
    Core::IO::FileSystem* fileSystem;

    mutable uint64_t nextID = 0;
    mutable Core::HashMap<Core::IO::Path, Core::OpaqueID<ASSET_TYPE>> assetNames;
    mutable Core::HashMap<Core::OpaqueID<ASSET_TYPE>, ASSET_TYPE*> assets;

protected:
    virtual ASSET_TYPE* create(Core::IO::InputStream& assetData, uint64_t ID) const = 0;
    virtual bool reload(Core::IO::InputStream& assetData, T& resource) const        = 0;

public:
    AssetCatalog(Core::IO::FileSystem* fileSystem) : fileSystem(fileSystem) {}

    Core::Status<ASSET_TYPE*> locate(const Core::IO::Path& assetPath) const {
        if(assetNames.count(assetPath) == 0) {
            ASSIGN_OR_ASSERT(Core::IO::InputStream assetData, fileSystem->openFile(assetPath));

            T* asset              = create(assetData, ++nextID);
            AssetTag<T> tag       = asset->tag;
            assets[tag]           = asset;
            assetNames[assetPath] = tag;

            Core::IO::WatchForChanges(assetPath, [=]() -> bool {
                ASSIGN_OR_ASSERT(Core::IO::InputStream assetData, fileSystem->OpenFile(assetPath));
                return this->reload(assetData, *resources[tag]);
            });
        }

        return assets[assetNames[assetPath]];
    }

    std::optional<ASSET_TYPE*> get(const AssetTag<ASSET_TYPE>& tag) const {
        auto asset = assets.find(tag);
        if(asset == assets.end()) {
            return std::nullopt;
        } else {
            return asset->second;
        }
    }
};
}    // namespace Assets