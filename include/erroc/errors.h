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
#define ERROC_EXPECT(condition,exception_type,...) \
    if(!(condition)) { \
        erroc::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__ __VA_OPT__(, __VA_ARGS__)); \
    }
#elif defined(_WIN32) || defined(_WIN64)
#define ERROC_EXPECT(condition,exception_type,...) \
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
#define _NTH_ARG(X101, X100, X99, X98, X97, X96, X95, X94, X93, X92, X91, X90, X89, X88, X87, X86, X85, X84, X83, X82, X81, X80, X79, X78, X77, X76, X75, X74, X73, X72, X71, X70, X69, X68, X67, X66, X65, X64, X63, X62, X61, X60, X59, X58, X57, X56, X55, X54, X53, X52, X51, X50, X49, X48, X47, X46, X45, X44, X43, X42, X41, X40, X39, X38, X37, X36, X35, X34, X33, X32, X31, X30, X29, X28, X27, X26, X25, X24, X23, X22, X21, X20, X19, X18, X17, X16, X15, X14, X13, X12, X11, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N,...) N
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)
#define _VA_NARGS(...) _NTH_ARG(_,##__VA_ARGS__,100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#elif defined(_WIN32) || defined(_WIN64)
#define _EXPAND(x) x
#define _NTH_ARG(X101, X100, X99, X98, X97, X96, X95, X94, X93, X92, X91, X90, X89, X88, X87, X86, X85, X84, X83, X82, X81, X80, X79, X78, X77, X76, X75, X74, X73, X72, X71, X70, X69, X68, X67, X66, X65, X64, X63, X62, X61, X60, X59, X58, X57, X56, X55, X54, X53, X52, X51, X50, X49, X48, X47, X46, X45, X44, X43, X42, X41, X40, X39, X38, X37, X36, X35, X34, X33, X32, X31, X30, X29, X28, X27, X26, X25, X24, X23, X22, X21, X20, X19, X18, X17, X16, X15, X14, X13, X12, X11, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N,...) N
#define _NARGS_1(...) _EXPAND(_NTH_ARG(__VA_ARGS__,100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define _AUGMENTER(...) unused,__VA_ARGS__

#define _VA_NARGS(...) _NARGS_1(_AUGMENTER(__VA_ARGS__))
#endif

#define ERROCPP_EXPECT(condition,exception_type,...) \
    {if (!(condition)) { \
        constexpr std::int64_t length{256}; \
        char buffer[length]; \
        erroc::details::Custom_streambuf<char> csb{ buffer, length }; \
        std::ostream os{&csb}; \
        erroc::details::format_error_prefix(os, #condition, #exception_type, __LINE__, __FUNCTION__, __FILE__); \
        if (_VA_NARGS(__VA_ARGS__) > 0) { \
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

            [[nodiscard]] const T& value() const
            {
                ERROC_EXPECT(has_value_, std::runtime_error, "value value not present");
                return value_;
            }

            [[nodiscard]] const E& error() const
            {
                ERROC_EXPECT(!has_value_, std::runtime_error, "error value not present");
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

