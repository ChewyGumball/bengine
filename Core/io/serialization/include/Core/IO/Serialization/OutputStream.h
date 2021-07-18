#pragma once

#include "Core/IO/Serialization/BinarySerializable.h"

#include <memory>
#include <span>
#include <string>

namespace Core::IO {

template <typename T>
struct Serializer {
    static void serialize(struct OutputStream& stream, const T& value);
};

template <typename T>
concept Serializable = requires(T a) {
    typename Serializer<T>;
};

struct OutputStream {
private:
    std::unique_ptr<class std::basic_ostream<std::byte>> stream;

public:
    OutputStream(std::basic_streambuf<std::byte>* stream);
    OutputStream(std::unique_ptr<class std::basic_ostream<std::byte>>&& stream);
    OutputStream(OutputStream&& other);

    template <Serializable T>
    void write(const T& value) {
        Serializer<T>::serialize(*this, value);
    }

    void write(std::span<const std::byte> data);
};

template <typename T>
requires std::is_trivially_copyable_v<T> struct Serializer<T> {
    static void serialize(OutputStream& stream, const T& value) {
        stream.write(std::as_bytes(std::span<const T>(&value, 1)));
    }
};

template <>
struct Serializer<std::string> {
    static void serialize(OutputStream& stream, const std::string& value) {
        stream.write(value.size());
        stream.write(std::as_bytes(std::span<const char>(value.data(), value.size())));
    }
};

}    // namespace Core::IO