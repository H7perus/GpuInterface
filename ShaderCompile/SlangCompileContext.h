#pragma once
#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

#include "ShaderCompile.h"
#include "SlangCompiledUnit.h"

#include "GpuInterface.h"

#include "GpuInterfaceDLL.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>

namespace KE::VK
{
	class GPUI_DLL_API SlangCompileContext
	{
		//TODO: global session should be split, so that a compute compiler and a graphics compiler can share the same global session. Anyway, problem for future me!
		Slang::ComPtr<slang::IGlobalSession> globalSession;

		Slang::ComPtr<slang::ISession> session;
		slang::SessionDesc sessionDesc;

		//not sure if we need multiple
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;

	public:
      SlangCompileContext();

      void LoadModule(char* modulecode)
      {
      }

		void LoadModulePath(char* path)
		{

		}

        void CompileShader(char* shadercode);

        SlangCompiledUnit CompileShaderPath(const char* path);
    };
}