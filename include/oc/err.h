#ifndef OC_ERR_H
#define OC_ERR_H

#include <variant>
#include <string>
#include <sstream>
#include <utility>
#include <stdexcept>

namespace oc {
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
#define OCERR_REQUIRE(condition, exception_type, ...) \
    if(!(condition)) { \
        std::string msg = oc::details::make_error_msg(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__ __VA_OPT__(, __VA_ARGS__)); \
        throw exception_type(msg); \
    }
#elif defined(_WIN32) || defined(_WIN64)
#define OCERR_REQUIRE(condition, exception_type, ...) \
    if(!(condition)) { \
        std::string msg = oc::details::make_error_msg(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__); \
        throw exception_type(msg); \
    }
#endif

namespace oc {
    namespace details {
        using nullopt_t = std::monostate;

        template <typename T = nullopt_t>
        class unexpected {
        public:
            constexpr unexpected(const T& value = nullopt_t{})
                : value_(value)
            {
            }
            constexpr unexpected(T&& value) noexcept
                : value_(std::move(value))
            {
            }
            constexpr unexpected(const unexpected&) = default;
            constexpr unexpected& operator=(const unexpected&) = default;
            constexpr unexpected(unexpected&&) = default;
            constexpr unexpected& operator=(unexpected&&) = default;
            ~unexpected() = default;

            [[nodiscard]] constexpr const T& value() const noexcept
            {
                return value_;
            }

        private:
            T value_;
        };

        template <typename T, typename E = nullopt_t>
        class expected {
        public:
            template <typename U = T>
            constexpr expected(const U& value)
                : opts_(value)
            {
            }
            template <typename U = T>
            constexpr expected(U&& value) noexcept
                : opts_(std::move(value))
            {
            }

            template <typename U = E>
            constexpr expected(const unexpected<U>& error)
                : opts_(error)
            {
            }
            template <typename U = E>
            constexpr expected(unexpected<U>&& error) noexcept
                : opts_(std::move(error))
            {
            }

            constexpr expected(const expected& other) = default;
            constexpr expected& operator=(const expected& other) = default;

            constexpr expected(expected&& other) = default;
            constexpr expected& operator=(expected&& other) = default;

            constexpr ~expected() = default;

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
                return std::get<unexpected<E>>(opts_).value();
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
                return decltype(op(value()))(unexpected(error()));
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
                return decltype(op())(unexpected(error()));
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
                    return expected<decltype(op(value())), E>(op(value()));
                }
                return expected<decltype(op(value())), E>(error());
            }

            template <typename Unary_op>
            [[nodiscard]] constexpr auto transform_error(Unary_op&& op) const
                requires (!std::is_same_v<E, nullopt_t>)
            {
                if (opts_.index() == 0) {
                    return expected<T, decltype(op(error()))>(value());
                }
                return expected<T, decltype(op(error()))>(op(error()));
            }

        private:
            constexpr expected() noexcept
                : expected(nullopt_t{})
            {
            }

            std::variant<T, unexpected<E>> opts_;
        };

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator==(const expected<T1, E1>& lhs, const expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() == rhs.value()) : (lhs.error() == rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator!=(const expected<T1, E1>& lhs, const expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return true;
            }

            return lhs ? (lhs.value() != rhs.value()) : (lhs.error() != rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator<(const expected<T1, E1>& lhs, const expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() < rhs.value()) : (lhs.error() < rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator<=(const expected<T1, E1>& lhs, const expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() <= rhs.value()) : (lhs.error() <= rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator>(const expected<T1, E1>& lhs, const expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() > rhs.value()) : (lhs.error() > rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline constexpr bool operator>=(const expected<T1, E1>& lhs, const expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() >= rhs.value()) : (lhs.error() >= rhs.error());
        }

        template <typename T>
        using optional = expected<T>;

        inline constexpr nullopt_t nullopt{};
    }

    using details::unexpected;
    using details::expected;
    using details::nullopt_t;
    using details::optional;
    using details::nullopt;
}

#endif // OC_ERR_H

