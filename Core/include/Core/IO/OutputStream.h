#pragma once

#include <ostream>
#include <streambuf>

namespace Core::IO {
using OutputByteStream = std::basic_ostream<std::byte>;

struct OutputStream;

template <typename T>
struct Serializer {
    static void serialize(OutputStream& stream, const T& value);
};

struct OutputStream {
private:
    std::unique_ptr<std::basic_ostream<std::byte>> stream;

public:
    OutputStream(std::basic_streambuf<std::byte>* stream)
      : stream(std::make_unique<OutputByteStream>(stream)) {}
    OutputStream(std::unique_ptr<OutputByteStream>&& stream) : stream(std::move(stream)) {}
    OutputStream(OutputStream&& other) : stream(std::move(other.stream)) {}

    template <typename T>
    void write(const T& value) {
        Serializer<T>::serialize(*this, value);
    }

    void write(const std::byte* data, uint64_t size) {
        stream->write(data, size);
    }
};


template <typename T>
void Serializer<T>::serialize(OutputStream& stream, const T& value) {
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);
    stream.write(reinterpret_cast<const std::byte*>(&value), sizeof(T));
}

template <>
struct Serializer<std::string> {
    static void serialize(OutputStream& stream, const std::string& value) {
        stream.write(value.size());
        stream.write(reinterpret_cast<const std::byte*>(value.data()), value.size() * sizeof(char));
    }
};

}    // namespace Core::IO