#ifndef MEMOC_ERRORS_H
#define MEMOC_ERRORS_H

#include <cstdarg>
#include <cstdint>
#include <cstdio>

namespace memoc {
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
#define MEMOC_THROW_IF_FALSE(condition,exception_type,...) \
    if(!(condition)) { \
        memoc::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__ __VA_OPT__(, __VA_ARGS__)); \
    }
#elif defined(_WIN32) || defined(_WIN64)
#define MEMOC_THROW_IF_FALSE(condition,exception_type,...) \
    if(!(condition)) { \
        memoc::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__); \
    }
#endif

#include <streambuf>
#include <ostream>

namespace memoc {
    namespace details {
        template <typename T>
        struct Custom_streambuf : public std::basic_streambuf<T, std::char_traits<T>> {
            Custom_streambuf(T* buffer, std::int64_t length)
            {
                this->setp(buffer, buffer + length);
            }

            std::int64_t written_size() const
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

#define MEMOCPP_THROW_IF_FALSE(condition,exception_type,...) \
    {if (!(condition)) { \
        constexpr std::int64_t length{256}; \
        char buffer[length]; \
        memoc::details::Custom_streambuf<char> csb{ buffer, length }; \
        std::ostream os{&csb}; \
        memoc::details::format_error_prefix(os, #condition, #exception_type, __LINE__, __FUNCTION__, __FILE__); \
        if (_VA_NARGS(__VA_ARGS__) > 0) { \
            os << " with message: " __VA_ARGS__; \
        } \
        std::int64_t wsize{ csb.written_size() }; \
        buffer[wsize < length ? wsize : length - 1] = '\0'; \
        throw exception_type{buffer}; \
    }}

namespace memoc {
    namespace details {
        template <typename T, typename E>
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

            Expected(const E& error)
                : error_(error), has_value_(false)
            {
            }
            Expected(E&& error) noexcept
                : error_(std::move(error)), has_value_(false)
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

            virtual ~Expected() = default;

            [[nodiscard]] explicit operator bool() const noexcept
            {
                return has_value_;
            }

            [[nodiscard]] const T& value() const
            {
                MEMOC_THROW_IF_FALSE(has_value_, std::runtime_error, "invalid value access");
                return value_;
            }

            [[nodiscard]] const E& error() const
            {
                MEMOC_THROW_IF_FALSE(!has_value_, std::runtime_error, "invalid error access");
                return error_;
            }

            [[nodiscard]] T value_or(const T& other) const
            {
                return has_value_ ? value_ : other;
            }
            [[nodiscard]] T value_or(T&& other) const
            {
                return has_value_ ? value_ : std::move(other);
            }

            template <typename Unary_op>
            [[nodiscard]] auto and_then(Unary_op op) const
            {
                if (has_value_) {
                    return Expected<decltype(op(value_)), E>(op(value_));
                }
                return Expected<decltype(op(value_)), E>(error_);
            }

            template <typename Unary_op>
            [[nodiscard]] auto and_then(Unary_op op) const requires std::is_void_v<decltype(op(Expected<T, E>{}.value())) >
            {
                if (has_value_) {
                    op(value_);
                    return Expected<T, E>(value_);
                }
                return *this;
            }

            template <typename Unary_op>
            [[nodiscard]] auto or_else(Unary_op op) const
            {
                if (has_value_) {
                    return Expected<T, decltype(op(error_))>(value_);
                }
                return Expected<T, decltype(op(error_))>(op(error_));
            }

            template <typename Unary_op>
            [[nodiscard]] auto or_else(Unary_op op) const requires std::is_void_v<decltype(op(Expected<T, E>{}.error())) >
            {
                if (has_value_) {
                    return *this;
                }
                op(error_);
                return Expected<T, E>(error_);
            }

        private:
            Expected() = default;

            union {
                T value_;
                E error_;
            };
            bool has_value_{ false };
        };


        struct Nullopt {
        };

        template <typename T>
        class Optional {
        public:
            Optional(const T& value) noexcept
                : value_(value), has_value_(true)
            {
            }
            Optional(T&& value) noexcept
                : value_(std::move(value)), has_value_(true)
            {
            }

            Optional(const Nullopt& value) noexcept
                : has_value_(false)
            {
            }

            Optional() noexcept
                : Optional(Nullopt{})
            {
            }

            Optional(const Optional&) = default;
            Optional& operator=(const Optional&) = default;
            Optional(Optional&&) = default;
            Optional& operator=(Optional&&) = default;
            virtual ~Optional() = default;

            [[nodiscard]] explicit operator bool() const noexcept {
                return has_value_;
            }

            [[nodiscard]] const T& value() const {
                MEMOC_THROW_IF_FALSE(has_value_, std::runtime_error, "invalid value access");
                return value_;
            }

            [[nodiscard]] T value_or(const T& other) const noexcept {
                return has_value_ ? value_ : other;
            }
            [[nodiscard]] T value_or(T&& other) const noexcept {
                return has_value_ ? value_ : std::move(other);
            }

        private:
            T value_;
            bool has_value_{ false };
        };
    }

    using details::Expected;

    using details::Nullopt;
    using details::Optional;
}

#endif // MEMOC_ERRORS_H

