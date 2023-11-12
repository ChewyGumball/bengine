#pragma once

#include "core/containers/Span.h"
#include "core/io/serialization/BinarySerializable.h"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <variant>

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

    void write(Core::Span<const std::byte> data);
    void write(Core::Span<std::byte> data);

    void writeText(const std::string_view text);

    template <Serializable T>
    void write(const T& value) {
        Serializer<T>::serialize(*this, value);
    }
};

template <typename T>
requires std::is_trivially_copyable_v<T> struct Serializer<T> {
    static void serialize(OutputStream& stream, const T& value) {
        stream.write(Core::AsBytes(Core::Span<const T>(&value, 1)));
    }
};

template <>
struct Serializer<std::string> {
    static void serialize(OutputStream& stream, const std::string& value) {
        stream.write(value.size());
        stream.write(Core::AsBytes(ToSpan(value)));
    }
};

template <>
struct Serializer<std::string_view> {
    static void serialize(OutputStream& stream, const std::string_view& value) {
        stream.write(value.size());
        stream.write(Core::AsBytes(ToSpan(value)));
    }
};

template <typename... Ts>
struct Serializer<std::variant<Ts...>> {
    static void serialize(OutputStream& stream, const std::variant<Ts...>& variant) {
        stream.write(variant.index());
        std::visit([&](const auto& value) { stream.write(value); }, variant);
    }
};

}    // namespace Core::IO