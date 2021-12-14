#ifndef XPP_UTILS_H
#define XPP_UTILS_H
#include "../libio/include/libio.h"

__attribute__((noreturn)) void die(const char* format, ...) noexcept;
__attribute__((noreturn)) void usage(FILE* stream) noexcept;
#endif // XPP_UTILS_H
