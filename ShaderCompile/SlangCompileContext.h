#pragma once
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"

#include "ShaderCompile.h"
#include "SlangCompiledUnit.h"

#include "GpuInterface.h"

#include "GpuInterfaceDLL.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <vector>

namespace KE::VK
{
class GPUI_DLL_API SlangCompileContext
{
    // TODO: global session should be split, so that a compute compiler and a graphics compiler can share the same
    // global session. Anyway, problem for future me!
    Slang::ComPtr<slang::IGlobalSession> globalSession;

  public:
    Slang::ComPtr<slang::ISession> session;
    slang::SessionDesc             sessionDesc;


    // not sure if we need multiple
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;

  public:
    // The resource heap stride baked into the SPIR-V comes from this device, so shaders index the heap the same way
    // the CPU fills it.
    explicit SlangCompileContext(u32 deviceIndex = 0);

    void LoadModule(char* modulecode)
    {
    }

    void LoadModulePath(char* path)
    {
    }

    void CompileShader(char* shadercode);

    SlangCompiledUnit CompileShaderPath(const char* path);
};
} // namespace KE::VK