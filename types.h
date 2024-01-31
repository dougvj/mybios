#ifndef TYPES_H
#define TYPES_H
#include <stddef.h>
#include <stdbool.h>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed long i32;
typedef signed long long i64;

#define packed __attribute__((packed))
#define unused __attribute__((unused))
#define noreturn __attribute__((noreturn))
#define section(x) __attribute__((section(x)))
#define aligned(x) __attribute__((aligned(x)))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


#undef NULL
#define NULL nullptr

#endif
