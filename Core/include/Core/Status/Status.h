#pragma once

#include <Core/Assert.h>

#include <spdlog/fmt/fmt.h>

#include <memory>
#include <string>

namespace Core {

class [[nodiscard]] Status {
public:
    Status(const Status& other) : inspected(false) {
        if(other.errorMessage) {
            errorMessage = std::make_unique<std::string>(*other.errorMessage);
        }
    }

    Status(Status && other) : inspected(other.inspected), errorMessage(std::move(other.errorMessage)) {
        other.inspected = true;
    }

    ~Status() {
        ASSERT_WITH_MESSAGE(inspected, "A Status must have isOk(), or isError() called on it before it is destroyed!");
    }

    void reset() const {
        inspected = false;
    }

    [[nodiscard]] bool isOk() const {
        inspected = true;
        return errorMessage == nullptr;
    }

    [[nodiscard]] bool isError() const {
        return !isOk();
    }

    bool peekError() const {
        return errorMessage != nullptr;
    }

    [[nodiscard]] const std::string& message() const& {
        ASSERT_WITH_MESSAGE(isError(), "A Status only has a message if it is in the error state!");
        return *errorMessage;
    }

    [[nodiscard]] std::string&& message()&& {
        ASSERT_WITH_MESSAGE(isError(), "A Status only has a message if it is in the error state!");
        return std::move(*errorMessage);
    }

    static Status Ok() {
        return Status(nullptr);
    }

    template <typename... FORMAT_ARGS>
    static Status Error(std::string_view errorMessage, FORMAT_ARGS && ... args) {
        return Status(std::make_unique<std::string>(fmt::format(errorMessage, std::forward<FORMAT_ARGS>(args)...)));
    }

private:
    Status(std::unique_ptr<std::string> errorMessage) : inspected(false), errorMessage(std::move(errorMessage)) {}

    mutable bool inspected;
    std::unique_ptr<std::string> errorMessage;
};
}    // namespace Core

#define RETURN_IF_ERROR_IMPL(tempName, status) \
    Status tempName(std::move(status));        \
    if(tempName.isError())                     \
        [[unlikely]] {                         \
            tempName.reset();                  \
            return std::move(tempName);        \
        }


#define RETURN_IF_ERROR(status) RETURN_IF_ERROR_IMPL(return_if_error_status_##__COUNTER__, status)

#define ASSERT_IS_OK_IMPL(tempName, status)                                                                    \
    Status tempName(std::move(status));                                                                        \
    if(tempName.IsError())                                                                                     \
        [[unlikely]] {                                                                                         \
            Core::AbortWithMessage("Status was expected to be ok, but it is an error: {}\n{} ({}) at {}:{}\n", \
                                   tempName.message(),                                                         \
                                   #status,                                                                    \
                                   CORE_ASSERT_STRINGIFY(status),                                              \
                                   __FILE__,                                                                   \
                                   __LINE__);                                                                  \
        }

#define ASSERT_IS_OK(status) ASSERT_IS_OK_IMPL(assert_is_ok_status_##__COUNTER__, status)
