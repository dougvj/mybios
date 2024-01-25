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
#define aligned(x) __attribute__((aligned(x)))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


#define GET_L(x) ((x)&0xFF)
#define GET_H(x) (((x) >> 8) & 0xFF)
#define SET_L(x, v) ((x) = ((x)&0xFF00) | ((v)&0xFF))
#define SET_H(x, v) ((x) = ((x)&0xFF) | (((u16)(v)&0xFF) << 8))
#define SET_LH(x, l, h) ((x) = (((u16)(h)&0xFF) << 8) | ((l)&0xFF))

#undef NULL
#define NULL nullptr

#endif
