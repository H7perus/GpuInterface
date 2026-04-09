#pragma once

#include "GpuInterface/BasicTypeAliases.h"
#include <string>

constexpr u64 FNV_OFFSET = 0xcbf29ce484222325ULL;
constexpr u64 FNV_PRIME  = 0x100000001b3ULL;


constexpr inline u64 FNV1aTo64(const char* input)
{
    u64 hash = FNV_OFFSET;
    for (int i = 0; input[i] != NULL; i++)
    {
        hash ^= input[i];
        hash *= FNV_PRIME;
    }
    return hash;
}