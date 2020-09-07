#pragma once

#include <istream>
#include <memory>
#include <span>
#include <string>
#include <type_traits>

namespace Core::IO {

template <typename T>
struct Deserializer {
    static T deserialize(struct InputStream& stream);
};

struct InputStream {
private:
    std::unique_ptr<std::basic_istream<std::byte>> stream;

public:
    InputStream(std::basic_streambuf<std::byte>* stream);
    InputStream(std::unique_ptr<std::basic_istream<std::byte>>&& stream);
    InputStream(InputStream&& other);

    template <typename T>
    T read() {
        return Deserializer<T>::deserialize(*this);
    }

    void readInto(std::byte* buffer, uint64_t size);

    template <typename T>
    void readInto(std::span<T> buffer) {
        static_assert(std::is_trivially_copyable_v<T>);
        readInto(reinterpret_cast<std::byte*>(buffer.data()), buffer.size() * sizeof(T));
    }
};

template <typename T>
T Deserializer<T>::deserialize(InputStream& stream) {
    static_assert(std::is_trivial_v<T>);

    T value;
    stream.readInto(reinterpret_cast<std::byte*>(&value), sizeof(T));
    return value;
}

template <>
struct Deserializer<std::string> {
    static std::string deserialize(InputStream& stream) {
        std::string value(stream.read<std::string::size_type>(), 0);
        stream.readInto(reinterpret_cast<std::byte*>(value.data()), value.size());
        return value;
    }
};

}    // namespace Core::IO