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

#endif // MEMOC_ERRORS_H

