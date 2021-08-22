#include "Assets/BufferLayout.h"

namespace Assets {

uint8_t Property::byteCount() const {
    size_t elementSize = PropertyType::SizeInBytes(type);
    return static_cast<uint8_t>(elementSize * elementCount);
}

bool operator==(const Property& a, const Property& b) {
    return a.type == b.type && a.elementCount == b.elementCount;
}

bool operator!=(const Property& a, const Property& b) {
    return !(a == b);
}

}    // namespace Assets