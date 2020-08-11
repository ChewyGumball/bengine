#pragma once

#include <Core/Status/Status.h>

#include <variant>

namespace Core {
template <typename T>
class StatusOr {
public:
    StatusOr(Status&& status) : valueOrStatus(std::move(status)) {
        ASSERT_WITH_MESSAGE(std::get<Status>(valueOrStatus).peekError(),
                            "StatusOr may only be constructed with a Status if that Status is in the error state!");
    }

    StatusOr(T value) : valueOrStatus(std::move(value)) {}

    [[nodiscard]] bool isOk() const {
        if(std::holds_alternative<Status>(valueOrStatus)) {
            return std::get<Status>(valueOrStatus).isOk();
        } else {
            return true;
        }
    }

    [[nodiscard]] bool isError() const {
        if(std::holds_alternative<Status>(valueOrStatus)) {
            return std::get<Status>(valueOrStatus).isError();
        } else {
            return false;
        }
    }

    bool peekError() const {
        if(std::holds_alternative<Status>(valueOrStatus)) {
            return std::get<Status>(valueOrStatus).peekError();
        } else {
            return false;
        }
    }

    [[nodiscard]] const std::string& message() const& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr only has a message if it holds an error Status!");

        return std::get<Status>(valueOrStatus).message();
    }

    [[nodiscard]] std::string&& message() && {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr only has a message if it holds an error Status!");

        return std::move(std::get<Status>(valueOrStatus)).message();
    }


    [[nodiscard]] const Status& status() const& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr doesn't have a status if it holds a value!");

        return std::get<Status>(valueOrStatus);
    }
    [[nodiscard]] Status&& status() && {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr doesn't have a status if it holds a value!");

        return std::move(std::get<Status>(valueOrStatus));
    }

    [[nodiscard]] const T& value() const& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<T>(valueOrStatus),
                            "A StatusOr doesn't have a value if it holds a Status!");

        return std::get<T>(valueOrStatus);
    }

    [[nodiscard]] T&& value() && {
        ASSERT_WITH_MESSAGE(std::holds_alternative<T>(valueOrStatus),
                            "A StatusOr doesn't have a value if it holds a Status!");

        return std::move(std::get<T>(valueOrStatus));
    }

private:
    std::variant<Status, T> valueOrStatus;
};
}    // namespace Core

#define ASSIGN_OR_RETURN_IMPL(tempName, lhs, rhs) \
    auto tempName((rhs));                         \
    if(tempName.peekError())                      \
        [[unlikely]] {                            \
            return std::move(tempName).status();  \
        }                                         \
    lhs = std::move(tempName).value();

#define ASSIGN_OR_RETURN(lhs, rhs) ASSIGN_OR_RETURN_IMPL(assign_or_return_status_or_##__COUNTER__, lhs, rhs)

#define ASSIGN_OR_ASSERT_IMPL(tempName, lhs, rhs)                                                         \
    auto tempName((rhs));                                                                                 \
    if(tempName.isError())                                                                                \
        [[unlikely]] {                                                                                    \
            Core::AbortWithMessage(                                                                       \
                  "StatusOr was expected to contain a value, but it is an error: {}\n{} ({}) at {}:{}\n", \
                  tempName.message(),                                                                     \
                  #rhs,                                                                                   \
                  CORE_ASSERT_STRINGIFY(rhs),                                                             \
                  __FILE__,                                                                               \
                  __LINE__);                                                                              \
        }                                                                                                 \
    lhs = std::move(tempName).value();

#define ASSIGN_OR_ASSERT(lhs, rhs) ASSIGN_OR_ASSERT_IMPL(assign_or_assert_status_or_##__COUNTER__, lhs, rhs)
