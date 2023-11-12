#pragma once

#include "core/containers/OpaqueID.h"

#include <variant>

namespace Assets {
template <typename ASSET_TYPE>
struct AssetReference {
    Core::IO::Path path;
};

template <typename ASSET_TYPE>
using AssetTag = Core::OpaqueID<ASSET_TYPE>;

}    // namespace Assets
