#pragma once

#include "core/io/serialization/BinarySerializable.h"

#include "core/containers/Span.h"
#include "core/status/Status.h"

#include <istream>
#include <memory>
#include <span>
#include <string>
#include <variant>

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

    uint64_t readInto(std::byte* buffer, uint64_t size);

    template <BinarySerializable T>
    void readInto(Core::Span<T> buffer) {
        uint64_t requiredSize = buffer.count() * sizeof(T);
        uint64_t actualSize   = readInto(reinterpret_cast<std::byte*>(buffer.rawData()), requiredSize);

        ASSERT_WITH_MESSAGE(requiredSize == actualSize,
                            "Expected to read {} bytes from the stream, but only {} were available",
                            requiredSize,
                            actualSize);
    }

    Core::Status rewind(uint64_t byteCount);
};

// Deserialize types which can just be read as bytes
template <BinarySerializable T>
struct Deserializer<T> {
    static T deserialize(InputStream& stream) {
        T value;

        uint64_t requiredSize = sizeof(T);
        uint64_t actualSize   = stream.readInto(reinterpret_cast<std::byte*>(&value), requiredSize);

        ASSERT_WITH_MESSAGE(requiredSize == actualSize,
                            "Expected to read {} bytes from the stream, but only {} were available",
                            requiredSize,
                            actualSize);

        return value;
    }
};

// Deserialize strings
template <>
struct Deserializer<std::string> {
    static std::string deserialize(InputStream& stream) {
        std::string value(stream.read<std::string::size_type>(), 0);
        stream.readInto(Core::AsWritableBytes(Core::ToSpan(value)));

        return value;
    }
};

// Deserialize variants
template <typename... Ts>
struct Deserializer<std::variant<Ts...>> {
    static std::variant<Ts...> deserialize(InputStream& stream) {
        std::size_t typeIndex = stream.read<std::size_t>();

        std::variant<Ts...> value;

        auto ReadIfTypeIndexIs = [&]<std::size_t INDEX>() {
            if(INDEX == typeIndex) {
                value = stream.read<std::variant_alternative_t<INDEX, std::variant<Ts...>>>();
            }
        };

        const auto indexSequence = std::make_index_sequence<std::variant_size_v<std::variant<Ts...>>>();

        [&]<std::size_t... N>(std::index_sequence<N...>) {
            ((ReadIfTypeIndexIs.operator()<N>()), ...);
        }
        (indexSequence);


        return value;
    }
};

}    // namespace Core::IO