#pragma once

namespace Assets {
template <typename ASSET_TYPE>
struct AssetTag {
    const uint32_t ID;

    AssetTag(uint32_t AssetID) : ID(AssetID) {}

    AssetTag() : this(0) {}
    AssetTag(const AssetTag<ASSET_TYPE>& other) : this(other.ID) {}
    AssetTag(const AssetTag<ASSET_TYPE>&& other) : this(other.ID) {}
};

template<typename ASSET_TYPE>
bool operator==(const AssetTag<ASSET_TYPE>& a, const AssetTag<ASSET_TYPE>& b) {
    return a.ID == b.ID;
}
template <typename ASSET_TYPE>
bool operator!=(const AssetTag<ASSET_TYPE>& a, const AssetTag<ASSET_TYPE>& b) {
    return a.ID != b.ID;
}
}    // namespace Assets

namespace std {

template <typename ASSET_TYPE>
struct hash<Assets::AssetTag<ASSET_TYPE>> {
    size_t operator()(const Assets::AssetTag<ASSET_TYPE>& x) const {
        return hash<uint32_t>()(x.ID);
    }
};
}