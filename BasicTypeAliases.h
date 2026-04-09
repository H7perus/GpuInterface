#pragma once

#include <cstdint>
#include <stdfloat>


//---------------------------------------------------------
//Type aliases for saner code.
//---------------------------------------------------------

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

//TODO WITH CPP23: This is bad! C++23 introduces fixed width floating point sizes. We should use those instead
//typedef float16_t f16;

typedef float f32;
typedef double f64;

typedef unsigned char byte;