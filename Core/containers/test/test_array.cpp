#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/Types.h"
#include "core/containers/Array.h"
#include "core/io/serialization/ArrayBuffer.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"

#include <type_traits>

struct Movable {
    i32 id;
    bool movedFrom;
    bool movedTo;
    Movable(i32 id) : id(id), movedFrom(false), movedTo(false) {}
    Movable(Movable&& other) : id(other.id), movedFrom(false), movedTo(true) {
        other.id        = -1;
        other.movedFrom = true;
    }
    Movable(Movable& other) = delete;

    bool operator==(const Movable& other) const {
        return other.id == id;
    }
};

std::ostream& operator<<(std::ostream& stream, const Movable& i) {
    return stream << i.id;
}

struct Copyable {
    i32 id;
    bool copiedFrom;
    bool copiedTo;
    Copyable(i32 id) : id(id), copiedFrom(false), copiedTo(false) {}
    Copyable(Copyable&& other) = delete;
    Copyable(Copyable& other) : id(other.id), copiedFrom(false), copiedTo(true) {
        other.copiedFrom = true;
    }

    bool operator==(const Copyable& other) const {
        return other.id == id;
    }
};

std::ostream& operator<<(std::ostream& stream, const Copyable& i) {
    return stream << i.id;
}

struct ConstCopyable {
    i32 id;
    mutable bool copiedFrom;
    bool copiedTo;
    ConstCopyable(i32 id) : id(id), copiedFrom(false), copiedTo(false) {}
    ConstCopyable(const ConstCopyable& other) : id(other.id), copiedFrom(false), copiedTo(true) {
        other.copiedFrom = true;
    }
    ConstCopyable(ConstCopyable&& other) = delete;

    bool operator==(const ConstCopyable& other) const {
        return other.id == id;
    }
};

std::ostream& operator<<(std::ostream& stream, const ConstCopyable& i) {
    return stream << i.id;
}

struct CopyMovable {
    i32 id;
    bool movedFrom;
    bool movedTo;
    bool copiedFrom;
    bool copiedTo;
    CopyMovable(i32 id) : id(id), movedFrom(false), movedTo(false), copiedFrom(false), copiedTo(false) {}
    CopyMovable(CopyMovable&& other)
      : id(other.id), movedFrom(false), movedTo(true), copiedFrom(false), copiedTo(false) {
        other.id        = -1;
        other.movedFrom = true;
    }
    CopyMovable(CopyMovable& other)
      : id(other.id), movedFrom(false), movedTo(false), copiedFrom(false), copiedTo(true) {
        other.copiedFrom = true;
    }

    bool operator==(const CopyMovable& other) const {
        return other.id == id;
    }
};

std::ostream& operator<<(std::ostream& stream, const CopyMovable& i) {
    return stream << i.id;
}

struct ConstCopyMovable {
    i32 id;
    bool movedFrom;
    bool movedTo;
    mutable bool copiedFrom;
    bool copiedTo;
    ConstCopyMovable(i32 id) : id(id), movedFrom(false), movedTo(false), copiedFrom(false), copiedTo(false) {}
    ConstCopyMovable(ConstCopyMovable&& other)
      : id(other.id), movedFrom(false), movedTo(true), copiedFrom(false), copiedTo(false) {
        other.id        = -1;
        other.movedFrom = true;
    }
    ConstCopyMovable(const ConstCopyMovable& other)
      : id(other.id), movedFrom(false), movedTo(false), copiedFrom(false), copiedTo(true) {
        other.copiedFrom = true;
    }

    bool operator==(const ConstCopyMovable& other) const {
        return other.id == id;
    }
};

std::ostream& operator<<(std::ostream& stream, const ConstCopyMovable& i) {
    return stream << i.id;
}


TEMPLATE_TEST_CASE("Insert Inline Move", "", Movable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(1));

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].movedTo);
    REQUIRE(array[0].id == 1);
}

TEMPLATE_TEST_CASE("Insert Inline Copy", "", Copyable, ConstCopyable) {
    Core::Array<TestType> array;

    array.insert(TestType(1));

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].copiedTo);
    REQUIRE(array[0].id == 1);
}

TEMPLATE_TEST_CASE("Insert Move", "", Movable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    TestType t(1);
    array.insert(std::move(t));

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].movedTo);
    REQUIRE(t.movedFrom);
    REQUIRE(t.id == -1);
    REQUIRE(array[0].id == 1);
}


TEMPLATE_TEST_CASE("Insert Copy", "", ConstCopyable, ConstCopyMovable) {
    Core::Array<TestType> array;

    TestType t(1);
    array.insert(t);

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].copiedTo);
    REQUIRE(t.copiedFrom);
    REQUIRE(t.id == 1);
    REQUIRE(array[0].id == 1);
}

TEMPLATE_TEST_CASE("Emplace Copy", "", Copyable, ConstCopyable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    TestType t(1);
    array.emplace(t);

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].copiedTo);
    REQUIRE(t.copiedFrom);
    REQUIRE(t.id == 1);
    REQUIRE(array[0].id == 1);
}

TEMPLATE_TEST_CASE("Emplace Move", "", Movable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    TestType t(1);
    array.emplace(std::move(t));

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].movedTo);
    REQUIRE(t.movedFrom);
    REQUIRE(t.id == -1);
    REQUIRE(array[0].id == 1);
}

TEMPLATE_TEST_CASE("Emplace Inline Move", "", Movable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.emplace(TestType(1));

    REQUIRE(array.count() == 1);
    REQUIRE(array[0].movedTo);
    REQUIRE(array[0].id == 1);
}

TEMPLATE_TEST_CASE("Resize Move", "", Movable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.emplace(TestType(0));

    TestType* firstElementPoi32er = array.begin();

    while(array.begin() == firstElementPoi32er) {
        array.emplace(TestType(array.count()));
    }

    for(const TestType& element : array) {
        REQUIRE(element.movedTo);
    }
}

TEMPLATE_TEST_CASE("Resize Copy", "", Copyable, ConstCopyable) {
    Core::Array<TestType> array;

    array.insert(TestType(0));

    TestType* firstElementPoi32er = array.begin();

    while(array.begin() == firstElementPoi32er) {
        array.insert(TestType(array.count()));
    }

    for(const TestType& element : array) {
        REQUIRE(element.copiedTo);
    }
}

TEST_CASE("No Arg Constructor") {
    Core::Array<u64> array;
    REQUIRE(array.count() == 0);
}

TEST_CASE("Initial Capacity Constructor") {
    Core::Array<u64> array(39);
    REQUIRE(array.count() == 0);
}

TEMPLATE_TEST_CASE("Repeating Constructor", "", u64, ConstCopyable, ConstCopyMovable) {
    constexpr u64 REPEAT_COUNT = 9;
    TestType VALUE(1);
    Core::Array<TestType> array(VALUE, REPEAT_COUNT);

    REQUIRE(array.count() == REPEAT_COUNT);
    for(u64 i = 0; i < REPEAT_COUNT; i++) {
        REQUIRE(array[i] == VALUE);
    }
}

TEST_CASE("Initializer List") {
    Core::Array<u64> array{1, 2, 3, 4, 5};

    REQUIRE(array.count() == 5);
    REQUIRE(array[0] == 1);
    REQUIRE(array[1] == 2);
    REQUIRE(array[2] == 3);
    REQUIRE(array[3] == 4);
    REQUIRE(array[4] == 5);
}

TEMPLATE_TEST_CASE("Copy Constructor", "", u64, ConstCopyable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));
    array.insert(TestType(4));
    array.insert(TestType(5));

    Core::Array<TestType> copy(array);

    array.insert(TestType(6));
    array.insert(TestType(7));
    array.insert(TestType(8));

    REQUIRE(array.count() == 8);
    REQUIRE(copy.count() == 5);

    for(u64 i = 0; i < copy.count(); i++) {
        REQUIRE(array[i] == copy[i]);
    }
}

TEMPLATE_TEST_CASE("Move Constructor", "", u64, Movable, Copyable, CopyMovable, ConstCopyable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));
    array.insert(TestType(4));
    array.insert(TestType(5));

    REQUIRE(array.count() == 5);

    Core::Array<TestType> copy(std::move(array));

    array.insert(TestType(6));
    array.insert(TestType(7));
    array.insert(TestType(8));

    REQUIRE(array.count() == 3);
    REQUIRE(copy.count() == 5);

    for(u64 i = 0; i < copy.count(); i++) {
        REQUIRE(copy[i] == TestType(i + 1));
    }
}

TEMPLATE_TEST_CASE("Copy Assignment", "", u64, ConstCopyable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));
    array.insert(TestType(4));
    array.insert(TestType(5));

    Core::Array<TestType> copy;
    copy = array;

    array.insert(TestType(6));
    array.insert(TestType(7));
    array.insert(TestType(8));

    REQUIRE(array.count() == 8);
    REQUIRE(copy.count() == 5);

    for(u64 i = 0; i < copy.count(); i++) {
        REQUIRE(array[i] == copy[i]);
    }
}

TEMPLATE_TEST_CASE("Move Assigmnet", "", u64, Movable, Copyable, CopyMovable, ConstCopyable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));
    array.insert(TestType(4));
    array.insert(TestType(5));

    REQUIRE(array.count() == 5);

    Core::Array<TestType> copy;
    copy = std::move(array);

    array.insert(TestType(6));
    array.insert(TestType(7));
    array.insert(TestType(8));

    REQUIRE(array.count() == 3);
    REQUIRE(copy.count() == 5);

    for(u64 i = 0; i < copy.count(); i++) {
        REQUIRE(copy[i] == TestType(i + 1));
    }
}

TEMPLATE_TEST_CASE("Insert At", "", u64, Movable, Copyable, ConstCopyable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(0));
    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));

    array.insertAt(1, TestType(4));

    REQUIRE(array.count() == 5);

    REQUIRE(array[0] == TestType(0));
    REQUIRE(array[1] == TestType(4));
    REQUIRE(array[2] == TestType(1));
    REQUIRE(array[3] == TestType(2));
    REQUIRE(array[4] == TestType(3));
}

TEMPLATE_TEST_CASE("Emplace At", "", u64, Movable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.emplace(TestType(0));
    array.emplace(TestType(1));
    array.emplace(TestType(2));
    array.emplace(TestType(3));

    array.emplaceAt(1, TestType(4));

    REQUIRE(array.count() == 5);

    REQUIRE(array[0] == TestType(0));
    REQUIRE(array[1] == TestType(4));
    REQUIRE(array[2] == TestType(1));
    REQUIRE(array[3] == TestType(2));
    REQUIRE(array[4] == TestType(3));
}

TEMPLATE_TEST_CASE("Erase At", "", u64, Movable, Copyable, ConstCopyable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(0));
    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));

    array.eraseAt(1);

    REQUIRE(array.count() == 3);

    REQUIRE(array[0] == TestType(0));
    REQUIRE(array[1] == TestType(2));
    REQUIRE(array[2] == TestType(3));
}


TEMPLATE_TEST_CASE("Clear", "", u64, Movable, Copyable, ConstCopyable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(0));
    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));

    REQUIRE(array.count() == 4);
    REQUIRE(!array.isEmpty());

    array.clear();

    REQUIRE(array.count() == 0);
    REQUIRE(array.isEmpty());
}

TEST_CASE("Insert Uninitialized") {
    constexpr u64 INSERT_COUNT = 8240;

    Core::Array<double> array;

    Core::Span<double> elements = array.insertUninitialized(INSERT_COUNT);

    REQUIRE(array.count() == INSERT_COUNT);
    REQUIRE(elements.count() == INSERT_COUNT);
}

TEST_CASE("Is Empty") {
    Core::Array<double> array;

    REQUIRE(array.isEmpty());
    array.insert(45.0);
    REQUIRE(!array.isEmpty());
}

TEMPLATE_TEST_CASE("Ensure Capacity", "", u64, Movable, Copyable, ConstCopyable, CopyMovable, ConstCopyMovable) {
    constexpr u64 ENSURED_CAPACITY = 637;

    Core::Array<TestType> array;

    array.ensureCapacity(ENSURED_CAPACITY);

    array.insert(TestType(0));

    TestType* firstElementPoi32er = array.begin();

    while(array.begin() == firstElementPoi32er) {
        array.insert(TestType(array.count()));
    }

    REQUIRE(array.count() > ENSURED_CAPACITY);
}

TEMPLATE_TEST_CASE("Insert All", "", u64, Copyable, ConstCopyable, CopyMovable, ConstCopyMovable) {
    Core::Array<TestType> array;

    array.insert(TestType(0));
    array.insert(TestType(1));
    array.insert(TestType(2));
    array.insert(TestType(3));

    Core::Array<TestType> array2;

    array2.insert(TestType(4));
    array2.insert(TestType(5));
    array2.insert(TestType(6));
    array2.insert(TestType(7));

    REQUIRE(array.count() == 4);
    REQUIRE(array2.count() == 4);

    array.insertAll(Core::ToSpan(array2));

    REQUIRE(array.count() == 8);
    REQUIRE(array2.count() == 4);

    for(u64 i = 0; i < array.count(); i++) {
        REQUIRE(array[i] == TestType(i));
    }
}

struct Trivial {
    i32 a;
    i32 b;

    Trivial() = default;
    Trivial(i32 a, i32 b) : a(a), b(b) {}
    bool operator==(const Trivial& other) const {
        return other.a == a && other.b == b;
    }
};

struct NonTrivial {
    Trivial a;
    Trivial b;

    NonTrivial() = default;
    NonTrivial(i32 a, i32 b) : a(a, b), b(b, a) {}
    NonTrivial(Trivial a, Trivial b) : a(a), b(b) {}
    NonTrivial(const NonTrivial& other) : a(other.a), b(other.b) {}

    bool operator==(const NonTrivial& other) const {
        return other.a == a && other.b == b;
    }
};

template <>
struct Core::IO::Serializer<NonTrivial> {
    static void serialize(OutputStream& stream, const NonTrivial& value) {
        stream.write(value.a);
        stream.write(value.b);
    }
};

template <>
struct Core::IO::Deserializer<NonTrivial> {
    static NonTrivial deserialize(InputStream& stream) {
        Trivial a = stream.read<Trivial>();
        Trivial b = stream.read<Trivial>();
        return NonTrivial(a, b);
    }
};

TEMPLATE_TEST_CASE("Serialize", "", Trivial, NonTrivial) {
    Core::Array<TestType> array;

    array.insert(TestType(1, 2));
    array.insert(TestType(3, 4));
    array.insert(TestType(5, 6));
    array.insert(TestType(7, 8));

    Core::IO::ArrayBuffer buffer;
    Core::IO::OutputStream out(&buffer);

    out.write(array);

    Core::IO::InputStream in(&buffer);
    Core::Array<TestType> result = in.read<Core::Array<TestType>>();

    REQUIRE(result.count() == array.count());
    for(u64 i = 0; i < array.count(); i++) {
        REQUIRE(result[i] == array[i]);
    }
}

TEST_CASE("No Implicit Integer Conversions") {
    REQUIRE(!std::is_convertible_v<u64, Core::Array<u64>>);
}
