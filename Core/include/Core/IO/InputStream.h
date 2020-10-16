#pragma once

#include <Core/IO/BinarySerializable.h>

#include <istream>
#include <memory>
#include <span>
#include <string>

namespace Core::IO {

template <typename T>
struct Deserializer {
    static T deserialize(struct InputStream& stream);
};

template <typename T>
concept Deserializable = requires(T a) {
    typename Deserializer<T>;
};

struct InputStream {
private:
    std::unique_ptr<std::basic_istream<std::byte>> stream;

public:
    InputStream(std::basic_streambuf<std::byte>* stream);
    InputStream(std::unique_ptr<std::basic_istream<std::byte>>&& stream);
    InputStream(InputStream&& other);

    template <Deserializable T>
    T read() {
        return Deserializer<T>::deserialize(*this);
    }

    void readInto(std::byte* buffer, uint64_t size);

    template <BinarySerializable T>
    void readInto(std::span<T> buffer) {
        readInto(reinterpret_cast<std::byte*>(buffer.data()), buffer.size() * sizeof(T));
    }
};

template <BinarySerializable T>
struct Deserializer<T> {
    static T deserialize(InputStream& stream) {
        T value;
        stream.readInto(reinterpret_cast<std::byte*>(&value), sizeof(T));
        return value;
    }
};

template <>
struct Deserializer<std::string> {
    static std::string deserialize(InputStream& stream) {
        std::string value(stream.read<std::string::size_type>(), 0);
        stream.readInto(reinterpret_cast<std::byte*>(value.data()), value.size());
        return value;
    }
};

}    // namespace Core::IO