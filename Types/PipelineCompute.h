#pragma once



#include "../ShaderCompile/SlangCompiledUnit.h"

#include "Pipeline.h"
//#include "DescriptorSetLayoutInfo.h"

#include "GpuInterface/GpuInterface.h"
#include "GpuInterfaceDLL.h"


#include "vulkan/vulkan.hpp"

namespace KE::VK
{
	class GPUI_DLL_API PipelineCompute : Pipeline
	{
	public:

		operator vk::Pipeline()
		{
			return pipeline_;
		}

		PipelineCompute();

        PipelineCompute(u32 deviceIndex, std::vector<uint32_t> spirv);

        PipelineCompute(u32 deviceIndex, SlangCompiledUnit shader);
    };
}