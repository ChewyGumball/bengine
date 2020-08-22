#pragma once

#include <cstdint>
#include <ostream>
#include <type_traits>

namespace Core {

template <typename LABEL, typename BASE_TYPE = uint64_t>
class OpaqueID {
    static_assert(std::is_integral_v<BASE_TYPE>());

    explicit OpaqueID(BASE_TYPE id) : id(id) {}

    BASE_TYPE value() const {
        return id;
    }

    bool operator==(const OpaqueID<LABEL, BASE_TYPE>& other) {
        return other.id == id;
    }

    bool operator!=(const OpaqueID<LABEL, BASE_TYPE>& other) {
        return !(*this == other);
    }

private:
    BASE_TYPE id;
};

template <typename LABEL, typename BASE_TYPE>
std::ostream& operator<<(std::ostream& stream, const Core::OpaqueID<LABEL, BASE_TYPE>& value) {
    return stream << value.value();
}

}    // namespace Core

namespace std {
template <typename LABEL, typename BASE_TYPE>
struct hash<Core::OpaqueID<LABEL, BASE_TYPE>> {
    size_t operator(const Core::OpaqueID<LABEL, BASE_TYPE>& value) const noexcept {
        return std::hash<BASE_TYPE>{}(value.value());
    }
};
}    // namespace std
