#pragma once

#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef s32 b32;

#define global_variable static
#define internal_func static // internal to translation unit
#define func_persist static
#define class_persist static

#define Pi32 3.14159265359f
#define PiOverTwo32 1.57079632679f
#define Tau32 6.28318530717958647692f
#define RadiansPerDegree (Pi32 / 180.0f)
#define U32_MAX ~0u

#define NANOSECONDS_PER_SECOND 1'000'000'000LL
#define MICROSECONDS_PER_SECOND 1'000'000L
#define MILLISECONDS_PER_SECOND 1000

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// Out is used to label out function parameters
#define Out

#ifdef NDEBUG
#define Assert(Expression)
#else
#define Assert(Expression) if(!(Expression)) *(u8*)0=0
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

typedef struct vec2_u32
{
    u32 width;
    u32 height;
} vec2_u32;

typedef struct vec2_f64 {
    f64 x;
    f64 y;
} vec2_f64;