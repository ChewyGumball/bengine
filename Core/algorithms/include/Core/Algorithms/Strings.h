#pragma once

#include <string_view>

#include "Core/Containers/Array.h"

namespace Core::Algorithms::String {

enum class Filter { None, Empty, Whitespace };

void SplitIntoBuffer(const std::string_view string,
                     char delimiter,
                     Core::Array<std::string_view>& buffer,
                     Filter filter = Filter::None);
Core::Array<std::string_view> Split(const std::string_view string, char delimiter, Filter filter = Filter::None);


void SplitIntoBuffer(const std::string_view string,
                     const std::string_view delimiters,
                     Core::Array<std::string_view>& buffer,
                     Filter filter = Filter::None);
Core::Array<std::string_view>
Split(const std::string_view string, std::string_view delimiters, Filter filter = Filter::None);


Core::Array<std::string_view> SplitLines(const std::string_view string, Filter filter = Filter::None);

int64_t ParseInt64(const std::string_view string);
uint64_t ParseUInt64(const std::string_view string);
float ParseFloat(const std::string_view string);
double ParseDouble(const std::string_view string);
}    // namespace Core::Algorithms::String