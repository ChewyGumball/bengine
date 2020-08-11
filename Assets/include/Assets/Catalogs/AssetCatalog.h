#pragma once

#include <filesystem>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

#include <Core/FileSystem/FileSystem.h>
#include <Core/FileSystem/InputStream.h>

#include "AssetTag.h"

namespace Assets {
template <typename ASSET_TYPE>
class AssetCatalog {
private:
    mutable uint32_t nextID = 0;
    mutable Core::HashMap<std::filesystem::path, AssetTag<ASSET_TYPE>> assetNames;
    mutable Core::HashMap<AssetTag<ASSET_TYPE>, ASSET_TYPE*> assets;

protected:
    virtual ASSET_TYPE* create(Core::FileSystem::InputStream& assetData, uint32_t ID) const = 0;
    virtual bool reload(Core::FileSystem::InputStream& assetData, T& resource) const        = 0;

public:
    std::optional<ASSET_TYPE*> locate(const std::filesystem::path& assetPath) const {
        if(assetNames.count(assetPath) == 0) {
            std::optional<Core::FileSystem::InputStream> assetData = Core::FileSystem::OpenFile(assetPath);
            ASSERT(assetData);

            T* asset              = create(assetData, ++nextID);
            AssetTag<T> tag       = asset->tag;
            assets[tag]           = asset;
            assetNames[assetPath] = tag;

            Core::FileSystem::WatchForChanges(assetPath, [=]() -> bool {
                std::optional<Core::FileSystem::InputStream> assetData = Core::FileSystem::OpenFile(assetPath);
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