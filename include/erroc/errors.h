#ifndef ERROC_ERRORS_H
#define ERROC_ERRORS_H

#include <cstdarg>
#include <cstdint>
#include <cstdio>

namespace erroc {
    namespace details {
        template <typename T, std::int64_t Buffer_size = 256>
        void format_and_throw(const char* condition, const char* exception_type, int line, const char* function, const char* file, const char* format = nullptr, ...)
        {
            char buffer[Buffer_size];

            int n = std::snprintf(buffer, Buffer_size, "'%s' failed on '%s' at line:%d@%s@%s", condition, exception_type, line, function, file);
            if (!format || *format == '\0' || n >= static_cast<int>(Buffer_size - 1)) {
                throw T{ buffer };
            }

            n += std::snprintf(buffer + n, Buffer_size - n, " with message: ");
            if (n < static_cast<int>(Buffer_size - 1)) {
                va_list args;
                va_start(args, format);
                n += std::vsnprintf(buffer + n, Buffer_size - n, format, args);
                va_end(args);
            }

            throw T{ buffer };
        }
    }
}

#ifdef __unix__
#define ERROC_EXPECT(condition, exception_type, ...) \
    if(!(condition)) { \
        erroc::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__ __VA_OPT__(, __VA_ARGS__)); \
    }
#elif defined(_WIN32) || defined(_WIN64)
#define ERROC_EXPECT(condition, exception_type, ...) \
    if(!(condition)) { \
        erroc::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__); \
    }
#endif

#include <streambuf>
#include <ostream>

namespace erroc {
    namespace details {
        template <typename T>
        struct Custom_streambuf : public std::basic_streambuf<T, std::char_traits<T>> {
            Custom_streambuf(T* buffer, std::int64_t length)
            {
                this->setp(buffer, buffer + length);
            }

            [[nodiscard]] std::int64_t written_size() const
            {
                return this->pptr() - this->pbase();
            }
        };

        inline void format_error_prefix(std::ostream& os, const char* condition, const char* exception_type, int line, const char* function, const char* file)
        {
            os << "'" << condition << "' failed on '" << exception_type << "' at line:" << line << "@" << function << "@" << file;
        }
    }
}

#ifdef __unix__
#define _ERROC_NTH_ARG(_101, _100, _99, _98, _97, _96, _95, _94, _93, _92, _91, _90, \
    _89, _88, _87, _86, _85, _84, _83, _82, _81, _80, \
    _79, _78, _77, _76, _75, _74, _73, _72, _71, _70, \
    _69, _68, _67, _66, _65, _64, _63, _62, _61, _60, \
    _59, _58, _57, _56, _55, _54, _53, _52, _51, _50, \
    _49, _48, _47, _46, _45, _44, _43, _42, _41, _40, \
    _39, _38, _37, _36, _35, _34, _33, _32, _31, _30, \
    _29, _28, _27, _26, _25, _24, _23, _22, _21, _20, \
    _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, \
    _9, _8, _7, _6, _5, _4, _3, _2, _1, n, ...) n
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)
#define _ERROC_VA_NARGS(...) _ERROC_NTH_ARG(_, ##__VA_ARGS__, \
    100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#elif defined(_WIN32) || defined(_WIN64)
#define _ERROC_EXPAND(x) x
#define _ERROC_NTH_ARG(_101, _100, _99, _98, _97, _96, _95, _94, _93, _92, _91, _90, \
    _89, _88, _87, _86, _85, _84, _83, _82, _81, _80, \
    _79, _78, _77, _76, _75, _74, _73, _72, _71, _70, \
    _69, _68, _67, _66, _65, _64, _63, _62, _61, _60, \
    _59, _58, _57, _56, _55, _54, _53, _52, _51, _50, \
    _49, _48, _47, _46, _45, _44, _43, _42, _41, _40, \
    _39, _38, _37, _36, _35, _34, _33, _32, _31, _30, \
    _29, _28, _27, _26, _25, _24, _23, _22, _21, _20, \
    _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, \
    _9, _8, _7, _6, _5, _4, _3, _2, _1, n, ...) n
#define _ERROC_NARGS_1(...) _ERROC_EXPAND(_ERROC_NTH_ARG(__VA_ARGS__, \
    100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define _ERROC_AUGMENT(...) unused, __VA_ARGS__

#define _ERROC_VA_NARGS(...) _ERROC_NARGS_1(_ERROC_AUGMENT(__VA_ARGS__))
#endif

#define ERROCPP_EXPECT(condition, exception_type, ...) \
    {if (!(condition)) { \
        constexpr std::int64_t length{256}; \
        char buffer[length]; \
        erroc::details::Custom_streambuf<char> csb{ buffer, length }; \
        std::ostream os{&csb}; \
        erroc::details::format_error_prefix(os, #condition, #exception_type, __LINE__, __FUNCTION__, __FILE__); \
        if (_ERROC_VA_NARGS(__VA_ARGS__) > 0) { \
            os << " with message: " __VA_ARGS__; \
        } \
        std::int64_t wsize{ csb.written_size() }; \
        buffer[wsize < length ? wsize : length - 1] = '\0'; \
        throw exception_type{buffer}; \
    }}

namespace erroc {
    namespace details {
        struct None_option {
        };

        [[nodiscard]] inline bool operator==(const None_option& lhs, const None_option& rhs)
        {
            return true;
        }

        template <typename T = None_option>
        class Unexpected {
        public:
            Unexpected(const T& value = None_option{})
                : value_(value)
            {
            }
            Unexpected(T&& value)
                : value_(std::move(value))
            {
            }
            Unexpected(const Unexpected&) = default;
            Unexpected& operator=(const Unexpected&) = default;
            Unexpected(Unexpected&&) = default;
            Unexpected& operator=(Unexpected&&) = default;
            virtual ~Unexpected() = default;

            [[nodiscard]] const T& value() const noexcept
            {
                return value_;
            }

        private:
            T value_;
        };

        template <typename T>
        concept Printable_error = requires(T t)
        {
            {to_string(t)};
        };

        template <typename T, typename E = None_option>
            requires (!std::is_same_v<None_option, T>)
        class Expected {
        public:
            Expected(const T& value)
                : value_(value), has_value_(true)
            {
            }
            Expected(T&& value) noexcept
                : value_(std::move(value)), has_value_(true)
            {
            }

            Expected(const Unexpected<E>& error)
                : error_(error.value()), has_value_(false)
            {
            }
            Expected(Unexpected<E>&& error) noexcept
                : error_(std::move(error.value())), has_value_(false)
            {
            }

            Expected(const Expected& other)
                : has_value_(other.has_value_)
            {
                if (other) {
                    value_ = other.value_;
                }
                else {
                    error_ = other.error_;
                }
            }
            Expected& operator=(const Expected& other)
            {
                if (&other == this) {
                    return *this;
                }

                Expected tmp(other);
                *this = std::move(tmp);

                return *this;
            }

            Expected(Expected&& other) noexcept
                : has_value_(other.has_value_)
            {
                if (other) {
                    value_ = std::move(other.value_);
                }
                else {
                    error_ = std::move(other.error_);
                }
            }
            Expected& operator=(Expected&& other) noexcept
            {
                if (&other == this) {
                    return *this;
                }

                has_value_ = other.has_value_;
                if (other) {
                    value_ = std::move(other.value_);
                }
                else {
                    error_ = std::move(other.error_);
                }

                return *this;
            }

            virtual ~Expected()
            {
                if (has_value_) {
                    value_.~T();
                }
                else {
                    error_.~E();
                }
            }

            [[nodiscard]] explicit operator bool() const noexcept
            {
                return has_value_;
            }

            [[nodiscard]] const T& value() const requires Printable_error<E>
            {
                ERROC_EXPECT(has_value_, std::runtime_error, "value is not present, error is '%s'", to_string(error_));
                return value_;
            }

            [[nodiscard]] const T& value() const
            {
                ERROC_EXPECT(has_value_, std::runtime_error, "value is not present");
                return value_;
            }

            [[nodiscard]] const E& error() const
            {
                ERROC_EXPECT(!has_value_, std::runtime_error, "error is not present");
                return error_;
            }

            template <typename U>
            [[nodiscard]] T value_or(const U& other) const
            {
                return has_value_ ? value_ : other;
            }
            template <typename U>
            [[nodiscard]] T value_or(U&& other) const
            {
                return has_value_ ? value_ : std::move(other);
            }

            template <typename Unary_op>
            [[nodiscard]] auto and_then(Unary_op&& op) const
            {
                if (has_value_) {
                    return Expected<decltype(op(value_)), E>(op(value_));
                }
                return Expected<decltype(op(value_)), E>(error_);
            }

            template <typename Unary_op>
            [[nodiscard]] auto and_then(Unary_op&& op) const requires std::is_void_v<decltype(op(Expected<T, E>{}.value())) >
            {
                if (has_value_) {
                    op(value_);
                    return Expected<T, E>(value_);
                }
                return *this;
            }

            template <typename Unary_op>
            [[nodiscard]] auto or_else(Unary_op&& op) const
            {
                if (has_value_) {
                    return Expected<T, decltype(op(error_))>(value_);
                }
                return Expected<T, decltype(op(error_))>(Unexpected<decltype(op(error_))>(op(error_)));
            }

            template <typename Unary_op>
            [[nodiscard]] auto or_else(Unary_op&& op) const requires std::is_void_v<decltype(op(Expected<T, E>{}.error())) >
            {
                if (has_value_) {
                    return *this;
                }
                op(error_);
                return Expected<T, E>(Unexpected<E>(error_));
            }

        private:
            Expected() noexcept
                : Expected(None_option{})
            {
            }

            union {
                T value_;
                E error_;
            };
            bool has_value_{ false };
        };

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline bool operator==(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() == rhs.value()) : (lhs.error() == rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline bool operator!=(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return true;
            }

            return lhs ? (lhs.value() != rhs.value()) : (lhs.error() != rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline bool operator<(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() < rhs.value()) : (lhs.error() < rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline bool operator<=(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() <= rhs.value()) : (lhs.error() <= rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline bool operator>(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() > rhs.value()) : (lhs.error() > rhs.error());
        }

        template <typename T1, typename E1, typename T2, typename E2>
        [[nodiscard]] inline bool operator>=(const Expected<T1, E1>& lhs, const Expected<T2, E2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.value() >= rhs.value()) : (lhs.error() >= rhs.error());
        }
    }

    using details::Unexpected;
    using details::Expected;
}

#endif // ERROC_ERRORS_H

