#pragma once

#include <string>

namespace Core::Algorithms::Mappers {
struct CharToString {
    std::string operator()(const char* c) const {
        return c;
    }
    std::string operator()(char* c) const {
        return c;
    }
};

struct StringToChar {
    const char* operator()(const std::string& s) const {
        return s.c_str();
    }
    const char* operator()(std::string& s) const {
        return s.c_str();
    }
};
}    // namespace Core::Algorithms::Mappers