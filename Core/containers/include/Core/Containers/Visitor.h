#pragma once

namespace Core {

template <typename... Ts>
struct Visitor : Ts... {
    using Ts::operator()...;
};

}    // namespace Core
