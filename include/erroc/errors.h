#ifndef ERROC_ERRORS_H
#define ERROC_ERRORS_H

#include <variant>
#include <string>
#include <sstream>
#include <utility>
#include <stdexcept>

namespace computoc {
    namespace details {
        inline std::string make_error_msg(const char* failed_cond, const char* exception_type, int line, const char* func, const char* file, const std::string& desc = std::string{})
        {
            std::stringstream ss;
            ss << exception_type << " exception (at line " << line << ", " << func << "@" << file << "), assertion " << failed_cond << " failed";
            if (!desc.empty()) {
                ss << ": " << desc;
            }
            return ss.str();
        }
    }
}

#ifdef __unix__
#define _REQUIRE(condition, exception_type, ...) \
    if(!(condition)) { \
        std::string msg = computoc::details::make_error_msg(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__ __VA_OPT__(, __VA_ARGS__)); \
        throw exception_type(msg); \
    }
#elif defined(_WIN32) || defined(_WIN64)
#define _REQUIRE(condition, exception_type, ...) \
    if(!(condition)) { \
        std::string msg = computoc::details::make_error_msg(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__); \
        throw exception_type(msg); \
    }
#endif

namespace computoc {

    namespace details {
        using None_option = std::monostate;

        template <typename T = None_option>
        class Unexpected {
        public:
            constexpr Unexpected(const T& value = None_option{})
                : value_(value)
            {
            }
            constexpr Unexpected(T&& value) noexcept
                : value_(std::move(value))
            {
            }
            constexpr Unexpected(const Unexpected&) = default;
            constexpr Unexpected& operator=(const Unexpected&) = default;
            constexpr Unexpected(Unexpected&&) = default;
            constexpr Unexpected& operator=(Unexpected&&) = default;
            ~Unexpected() = default;

            [[nodiscard]] constexpr const T& value() const noexcept
            {
                return value_;
            }

        private:
            T value_;
        };

        template <typename T, typename E = None_option>
        //requires (!std::is_same_v<None_option, T>)
        class Expected {
        public:
            template <typename U = T>
            constexpr Expected(const U& value)
                : opts_(value)
            {
            }
            template <typename U = T>
            constexpr Expected(U&& value) noexcept
                : opts_(std::move(value))
            {
            }

            template <typename U = E>
            constexpr Expected(const Unexpected<U>& error)
                : opts_(error)
            {
            }
            template <typename U = E>
            constexpr Expected(Unexpected<U>&& error) noexcept
                : opts_(std::move(error))
            {
            }

            constexpr Expected(const Expected& other) = default;
            constexpr Expected& operator=(const Expected& other) = default;

            constexpr Expected(Expected&& other) = default;
            constexpr Expected& operator=(Expected&& other) = default;

            constexpr ~Expected() = default;

            [[nodiscard]] explicit constexpr operator bool() const noexcept
            {
                return opts_.index() == 0;
            }

            [[nodiscard]] constexpr bool has_value() const noexcept
            {
                return opts_.index() == 0;
            }

            [[nodiscard]] constexpr const T& value() const
            {
                if (opts_.index() == 1) {
                    throw std::runtime_error("value is not present");
                }
                return std::get<T>(opts_);
            }

            [[nodiscard]] constexpr const T& operator*() const
            {
                if (opts_.index() == 1) {
                    throw std::runtime_error("value is not present");
                }
                return std::get<T>(opts_);
            }

            [[nodiscard]] constexpr const T* operator->() const
            {
                if (opts_.index() == 1) {
                    throw std::runtime_error("value is not present");
                }
                return &std::get<T>(opts_);
            }

            [[nodiscard]] constexpr const E& error() const
            {
                if (opts_.index() == 0) {
                    throw std::runtime_error("error is not present");
                }
                return std::get<Unexpected<E>>(opts_).value();
            }

            template <typename U>
            [[nodiscard]] constexpr T value_or(const U& other) const
            {
                return opts_.index() == 0 ? std::get<T>(opts_) : other;
            }
            template <typename U>
            [[nodiscard]] constexpr T value_or(U&& other) const
            {
                return opts_.index() == 0 ? std::get<T>(opts_) : std::move(other);
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto and_then(Unary_op&& op) const
            {
                if (opts_.index() == 0) {
                    return op(value());
                }
                return decltype(op(value()))(Unexpected(error()));
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto and_then(Unary_op&& op) const requires std::is_void_v<decltype(op(value()))>
            {
                if (opts_.index() == 0) {
                    op(value());
                }
                return *this;
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto and_then(Unary_op&& op) const requires std::is_invocable_v<Unary_op>
            {
                if (opts_.index() == 0) {
                    return op();
                }
                return decltype(op())(Unexpected(error()));
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto or_else(Unary_op&& op) const
            {
                if (opts_.index() == 0) {
                    return decltype(op(error()))(value());
                }
                return op(error());
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto or_else(Unary_op&& op) const requires std::is_void_v<decltype(op(error()))>
            {
                if (opts_.index() == 0) {
                    return *this;
                }
                op(error());
                return *this;
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto or_else(Unary_op&& op) const requires std::is_invocable_v<Unary_op>
            {
                if (opts_.index() == 0) {
                    return decltype(op())(value());
                }
                return op();
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto transform(Unary_op&& op) const
            {
                if (opts_.index() == 0) {
                    return Expected<decltype(op(value())), E>(op(value()));
                }
                return Expected<decltype(op(value())), E>(error());
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto transform_error(Unary_op&& op) const
            {
                if (opts_.index() == 0) {
                    return Expected<T, decltype(op(error()))>(value());
                }
                return Expected<T, decltype(op(error()))>(op(error()));
            }

        private:
            constexpr Expected() noexcept
                : Expected(None_option{})
            {
            }

            std::variant<T, Unexpected<E>> opts_;
        };

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator==(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() == rhs.value()) : (lhs.error() == rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator!=(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return true;
            }

            return lhs ? (lhs.value() != rhs.value()) : (lhs.error() != rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator<(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() < rhs.value()) : (lhs.error() < rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator<=(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() <= rhs.value()) : (lhs.error() <= rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator>(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() > rhs.value()) : (lhs.error() > rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator>=(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() >= rhs.value()) : (lhs.error() >= rhs.error());
        }
    }

    using details::Unexpected;
    using details::Expected;
    using details::None_option;
}

#endif // ERROC_ERRORS_H

