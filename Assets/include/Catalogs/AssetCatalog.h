#pragma once

#include <filesystem>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

#include "AssetTag.h"

namespace Assets {
template <typename ASSET_TYPE>
class AssetCatalog {
private:
    mutable uint32_t nextID = 0;
    mutable Core::HashMap<std::string, AssetTag<ASSET_TYPE>> assetNames;
    mutable Core::HashMap<AssetTag<ASSET_TYPE>, ASSET_TYPE*> assets;

protected:
    virtual ASSET_TYPE* create(const std::filesystem::path& assetPath, uint32_t ID) const = 0;
    virtual bool reload(const std::filesystem::path& assetPath, T& resource) const        = 0;

public:
    AssetCatalog() {}

    std::optional<ASSET_TYPE*> locate(const std::filesystem::path& assetPath) const {
        if(assetNames.count(assetPath) == 0) {
            bool found = false;
            for(auto& path : resourceLocations) {
                std::string location = path + "/" + resourceName;
                if(std::ifstream(location)) {
                    T* resource                 = create(location, ++nextID);
                    ResourceTag<T> tag          = resource->tag;
                    resources[tag]              = resource;
                    resourceNames[resourceName] = tag;

                    Util::File::WatchForChanges(location,
                                                [=]() -> bool { return this->reload(location, *resources[tag]); });
                    found = true;
                    break;
                }
            }

            assert(found);
        }

        return resources[resourceNames[resourceName]];

    }

    std::optional<ASSET_TYPE*> get(const AssetTag<ASSET_TYPE>& tag) const {
        auto resource = resources.find(tag);
        if(resource == resources.end()) {
            return std::nullopt;
        } else {
            return resource->second;
        }
    }
};
}    // namespace Assets