#pragma once

#include <string_view>

#include "Core/Containers/Array.h"
#include "Core/DllExport.h"

namespace Core::Algorithms::String {

enum class Filter { None, Empty, Whitespace };

CORE_API void SplitIntoBuffer(const std::string_view string,
                              char delimiter,
                              Core::Array<std::string_view>& buffer,
                              Filter filter = Filter::None);
CORE_API Core::Array<std::string_view>
Split(const std::string_view string, char delimiter, Filter filter = Filter::None);


CORE_API void SplitIntoBuffer(const std::string_view string,
                              const std::string_view delimiters,
                              Core::Array<std::string_view>& buffer,
                              Filter filter = Filter::None);
CORE_API Core::Array<std::string_view>
Split(const std::string_view string, std::string_view delimiters, Filter filter = Filter::None);


CORE_API Core::Array<std::string_view> SplitLines(const std::string_view string, Filter filter = Filter::None);

CORE_API int64_t ParseInt64(const std::string_view string);
CORE_API uint64_t ParseUInt64(const std::string_view string);
CORE_API float ParseFloat(const std::string_view string);
CORE_API double ParseDouble(const std::string_view string);
}    // namespace Core::Algorithms::String