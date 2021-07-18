#pragma once

#include <Core/Status/Status.h>

#include <variant>

namespace Core {
template <typename T>
requires !std::is_same_v<T, Status> class [[nodiscard]] StatusOr {
public:
    StatusOr(Status && status) : valueOrStatus(std::move(status)) {
        ASSERT_WITH_MESSAGE(std::get<Status>(valueOrStatus).peekError(),
                            "StatusOr may only be constructed with a Status if that Status is in the error state!");
    }

    StatusOr(T value) : valueOrStatus(std::move(value)) {}

    [[nodiscard]] bool isOk() const {
        if(std::holds_alternative<Status>(valueOrStatus)) {
            // We call this function on the status so that it gets its "inspected" state set properly. Otherwise
            // inspecting the StatusOr will not count as inspecting the Status.
            return std::get<Status>(valueOrStatus).isOk();
        } else {
            return true;
        }
    }

    [[nodiscard]] bool isError() const {
        if(std::holds_alternative<Status>(valueOrStatus)) {
            // We call this function on the status so that it gets its "inspected" state set properly. Otherwise
            // inspecting the StatusOr will not count as inspecting the Status.
            return std::get<Status>(valueOrStatus).isError();
        } else {
            return false;
        }
    }

    [[nodiscard]] bool peekError() const {
        return std::holds_alternative<Status>(valueOrStatus);
    }

    [[nodiscard]] const std::string& message() const& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr only has a message if it holds an error Status!");

        return std::get<Status>(valueOrStatus).message();
    }

    [[nodiscard]] std::string&& message()&& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr only has a message if it holds an error Status!");

        return std::move(std::get<Status>(valueOrStatus)).message();
    }


    [[nodiscard]] const Status& status() const& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr doesn't have a status if it holds a value!");

        return std::get<Status>(valueOrStatus);
    }
    [[nodiscard]] Status&& status()&& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<Status>(valueOrStatus),
                            "A StatusOr doesn't have a status if it holds a value!");

        return std::move(std::get<Status>(valueOrStatus));
    }

    [[nodiscard]] const T& value() const& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<T>(valueOrStatus),
                            "A StatusOr doesn't have a value if it holds a Status!");

        return std::get<T>(valueOrStatus);
    }

    [[nodiscard]] T& value()& {
        ASSERT_WITH_MESSAGE(std::holds_alternative<T>(valueOrStatus),
                            "A StatusOr doesn't have a value if it holds a Status!");

        return std::get<T>(valueOrStatus);
    }

    [[nodiscard]] T&& value()&& {
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

#define ASSIGN_OR_RETURN(lhs, rhs) \
    ASSIGN_OR_RETURN_IMPL(CORE_ASSERT_CONCAT(assign_or_return_status_or_, __COUNTER__), lhs, rhs)

#define CONSTRUCT_OR_RETURN_IMPL(tempName, lhs, rhs) \
    auto tempName((rhs));                            \
    if(tempName.peekError())                         \
        [[unlikely]] {                               \
            return std::move(tempName).status();     \
        }                                            \
    lhs(std::move(tempName).value());

#define CONSTRUCT_OR_RETURN(lhs, rhs) \
    CONSTRUCT_OR_RETURN_IMPL(CORE_ASSERT_CONCAT(construct_or_return_status_or_, __COUNTER__), lhs, rhs)

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

#define ASSIGN_OR_ASSERT(lhs, rhs) \
    ASSIGN_OR_ASSERT_IMPL(CORE_ASSERT_CONCAT(assign_or_assert_status_or_, __COUNTER__), lhs, rhs)
