#pragma once

#include "BasicTypeAliases.h"

#include <string>
#include <vulkan/vulkan.hpp>

namespace KE::VK
{
    struct VertexInput
    {
        std::string semanticName;
        u32  semanticIndex;
        vk::Format format;
        u32  location;
    };

    struct StructMember
    {
        std::string name;
        std::string type;
        uint32_t   offset;
    };

    struct CompileTimeConstant
    {
        std::string name;
        i32         def = 0;
        i32         min = -1;
        i32         max = -1;
    };

    struct CompileTimeConstantValue
    {
        std::string name;
        i32 value = 0;
    };
}