#pragma once

#include <string_view>
#include <vector>

#include <Core/DllExport.h>

namespace Core::Algorithms::String {

enum class Filter { None, Empty, Whitespace };
CORE_API std::vector<std::string_view>
Split(const std::string_view string, char delimiter, Filter filter = Filter::None);

CORE_API int64_t ParseInt64(const std::string_view string);
CORE_API uint64_t ParseUInt64(const std::string_view string);
CORE_API float ParseFloat(const std::string_view string);
CORE_API double ParseDouble(const std::string_view string);
}    // namespace Core::Algorithms::String