#pragma once

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>
#include <Core/Containers/OpaqueID.h>
#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/Status/Status.h>

#include "AssetsReference.h"

#include <memory>

namespace Assets {
template <typename ASSET_TYPE>
class AssetCatalog {
private:
    Core::IO::FileSystem* fileSystem;

    mutable uint64_t nextID = 0;
    mutable Core::HashMap<Core::IO::Path, AssetTag<ASSET_TYPE>> resolvedReferences;
    mutable Core::HashMap<AssetTag<ASSET_TYPE>, std::unique_ptr<ASSET_TYPE>> assets;

protected:
    virtual Core::StatusOr<ASSET_TYPE*> create(Core::IO::InputStream& assetData) const = 0;
    virtual Core::Status reload(Core::IO::InputStream& assetData, T& resource) const   = 0;

public:
    struct ResolveResult {
        AssetTag<ASSET_TYPE> tag;
        ASSET_TYPE* asset;
    };

    AssetCatalog(Core::IO::FileSystem* fileSystem) : fileSystem(fileSystem) {}

    Core::StatusOr<ResolveResult> resolve(const AssetReference<ASSET_TYPE>& reference) const {
        AssetTag<ASSET_TYPE> tag;

        auto tagIterator = resolvedReferences.find(reference.path);
        if(tagIterator == resolvedReferences.end()) {
            ASSIGN_OR_RETURN(Core::IO::InputStream assetData, fileSystem->openFile(reference.path));

            ASSIGN_OR_RETURN(ASSET_TYPE * asset, create(assetData));
            tag = AssetTag<ASSET_TYPE>(nextID++);

            assets.emplace(tag, asset);

            filesystem->WatchForChanges(reference.path, [=]() -> bool {
                ASSIGN_OR_RETURN(Core::IO::InputStream assetData, fileSystem->OpenFile(reference.path));
                return this->reload(assetData, *assets[tag]);
            });
        } else {
            tag = tagIterator->second;
        }

        return ResolveResult{.tag = tag, .asset = assets[tag].get()};
    }


    Core::StatusOr<ASSET_TYPE*> get(const AssetTag<ASSET_TYPE>& tag) const {
        auto asset = assets.find(tag);
        if(asset == assets.end()) {
            return Core::Status::Error("Could not find asset with tag {}", tag);
        } else {
            return asset->get();
        }
    }

    void remove(const AssetReference<ASSET_TYPE>& reference) const {
        auto referenceIterator = resolvedReferences.find(reference.path);
        if(referenceIterator != resolvedReferences.end()) {
            assets.erase(referenceIterator->second);
            resolvedReferences.erase(referenceIterator);
        }
    }
};
}    // namespace Assets