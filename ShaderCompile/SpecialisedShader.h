#pragma once
#include "GpuInterface/ShaderCompile/ShaderReflection.h"
#include <vector>

namespace KE::VK
{
    struct SpecializedShader
    {
        std::vector<uint32_t> spirv;
        std::vector<VertexInput> vertexInputs;
        std::vector<StructMember> pushConstants;
        std::vector<StructMember> materialParameters;
        std::vector<CompileTimeConstantValue> compileTimeConstants;
    };
}