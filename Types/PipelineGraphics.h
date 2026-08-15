#pragma once
#include "Pipeline.h"

#include "GpuInterface/GpuInterface.h"

#include "GpuInterfaceDLL.h"
#include "vulkan/vulkan.hpp"

namespace KE::VK
{
	class GPUI_DLL_API PipelineGraphics : public Pipeline
	{
	public:
		PipelineGraphics() {};
		PipelineGraphics(u32 deviceIndex, SpecializedShader& shader);
		PipelineGraphics(u32 deviceIndex, std::vector<uint32_t> vertSpirv, std::vector<uint32_t> fragSpirv);
	};
}


