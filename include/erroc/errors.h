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
#define _NTH_ARG(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)
#define _VA_NARGS(...) _NTH_ARG(_,##__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
#elif defined(_WIN32) || defined(_WIN64)
#define _EXPAND(x) x
#define _NTH_ARG(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define _NARGS_1(...) _EXPAND(_NTH_ARG(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0))
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
        struct Null_option {
        };

        template <typename T1, typename T2 = Null_option>
        class Optional {
        public:
            Optional(const T1& expected)
                : expected_(expected), has_expected_(true)
            {
            }
            Optional(T1&& expected) noexcept
                : expected_(std::move(expected)), has_expected_(true)
            {
            }

            Optional(const T2& unexpected)
                : unexpected_(unexpected), has_expected_(false)
            {
            }
            Optional(T2&& unexpected) noexcept
                : unexpected_(std::move(unexpected)), has_expected_(false)
            {
            }

            Optional() noexcept
                : Optional(Null_option{})
            {
            }

            Optional(const Optional& other)
                : has_expected_(other.has_expected_)
            {
                if (other) {
                    expected_ = other.expected_;
                }
                else {
                    unexpected_ = other.unexpected_;
                }
            }
            Optional& operator=(const Optional& other)
            {
                if (&other == this) {
                    return *this;
                }

                Optional tmp(other);
                *this = std::move(tmp);

                return *this;
            }

            Optional(Optional&& other) noexcept
                : has_expected_(other.has_expected_)
            {
                if (other) {
                    expected_ = std::move(other.expected_);
                }
                else {
                    unexpected_ = std::move(other.unexpected_);
                }
            }
            Optional& operator=(Optional&& other) noexcept
            {
                if (&other == this) {
                    return *this;
                }

                has_expected_ = other.has_expected_;
                if (other) {
                    expected_ = std::move(other.expected_);
                }
                else {
                    unexpected_ = std::move(other.unexpected_);
                }

                return *this;
            }

            virtual ~Optional() = default;

            [[nodiscard]] explicit operator bool() const noexcept
            {
                return has_expected_;
            }

            [[nodiscard]] const T1& expected() const
            {
                ERROC_EXPECT(has_expected_, std::runtime_error, "expected value not present");
                return expected_;
            }

            [[nodiscard]] const T2& unexpected() const
            {
                ERROC_EXPECT(!has_expected_, std::runtime_error, "unexpected value not present");
                return unexpected_;
            }

            template <typename U>
            [[nodiscard]] T1 expected_or(const U& other) const
            {
                return has_expected_ ? expected_ : other;
            }
            template <typename U>
            [[nodiscard]] T1 expected_or(U&& other) const
            {
                return has_expected_ ? expected_ : std::move(other);
            }

            template <typename Unary_op>
            [[nodiscard]] auto and_then(Unary_op&& op) const
            {
                if (has_expected_) {
                    return Optional<decltype(op(expected_)), T2>(op(expected_));
                }
                return Optional<decltype(op(expected_)), T2>(unexpected_);
            }

            template <typename Unary_op>
            [[nodiscard]] auto and_then(Unary_op&& op) const requires std::is_void_v<decltype(op(Optional<T1, T2>{}.expected())) >
            {
                if (has_expected_) {
                    op(expected_);
                    return Optional<T1, T2>(expected_);
                }
                return *this;
            }

            template <typename Unary_op>
            [[nodiscard]] auto or_else(Unary_op&& op) const
            {
                if (has_expected_) {
                    return Optional<T1, decltype(op(unexpected_))>(expected_);
                }
                return Optional<T1, decltype(op(unexpected_))>(op(unexpected_));
            }

            template <typename Unary_op>
            [[nodiscard]] auto or_else(Unary_op&& op) const requires std::is_void_v<decltype(op(Optional<T1, T2>{}.unexpected())) >
            {
                if (has_expected_) {
                    return *this;
                }
                op(unexpected_);
                return Optional<T1, T2>(unexpected_);
            }

        private:
            union {
                T1 expected_;
                T2 unexpected_;
            };
            bool has_expected_{ false };
        };

        template <typename T1, typename T2, typename U1, typename U2>
        [[nodiscard]] inline bool operator==(const Optional<T1, T2>& lhs, const Optional<U1, U2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.expected() == rhs.expected()) : (lhs.unexpected() == rhs.unexpected());
        }

        template <typename T1, typename T2, typename U1, typename U2>
        [[nodiscard]] inline bool operator!=(const Optional<T1, T2>& lhs, const Optional<U1, U2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return true;
            }

            return lhs ? (lhs.expected() != rhs.expected()) : (lhs.unexpected() != rhs.unexpected());
        }

        template <typename T1, typename T2, typename U1, typename U2>
        [[nodiscard]] inline bool operator<(const Optional<T1, T2>& lhs, const Optional<U1, U2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.expected() < rhs.expected()) : (lhs.unexpected() < rhs.unexpected());
        }

        template <typename T1, typename T2, typename U1, typename U2>
        [[nodiscard]] inline bool operator<=(const Optional<T1, T2>& lhs, const Optional<U1, U2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.expected() <= rhs.expected()) : (lhs.unexpected() <= rhs.unexpected());
        }

        template <typename T1, typename T2, typename U1, typename U2>
        [[nodiscard]] inline bool operator>(const Optional<T1, T2>& lhs, const Optional<U1, U2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.expected() > rhs.expected()) : (lhs.unexpected() > rhs.unexpected());
        }

        template <typename T1, typename T2, typename U1, typename U2>
        [[nodiscard]] inline bool operator>=(const Optional<T1, T2>& lhs, const Optional<U1, U2>& rhs)
        {
            if (static_cast<bool>(lhs) != static_cast<bool>(rhs)) {
                return false;
            }

            return lhs ? (lhs.expected() >= rhs.expected()) : (lhs.unexpected() >= rhs.unexpected());
        }
    }

    using details::Null_option;
    using details::Optional;
}

#endif // ERROC_ERRORS_H

