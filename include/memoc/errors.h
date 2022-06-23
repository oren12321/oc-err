#ifndef MEMOC_ERRORS_H
#define MEMOC_ERRORS_H

#include <cstdarg>
#include <cstddef>
#include <cstdio>

namespace memoc::errors {
    namespace details {
        template <typename T, std::size_t Buffer_size = 256>
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
#define THROW_IF_FALSE(condition,exception_type,...) \
    if(!(condition)) { \
        memoc::errors::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__ __VA_OPT__(, __VA_ARGS__)); \
    }
#elif defined(_WIN32) || defined(_WIN64)
#define THROW_IF_FALSE(condition,exception_type,...) \
    if(!(condition)) { \
        memoc::errors::details::format_and_throw<exception_type>(#condition, #exception_type, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__); \
    }
#endif

#endif // MEMOC_ERRORS_H

