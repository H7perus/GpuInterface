#pragma once

#include "../ShaderCompile/SlangCompiledUnit.h"

#include "vulkan/vulkan.hpp"


namespace KE::VK
{
	class Pipeline
	{
	protected:
		vk::Pipeline pipeline_;
		u32 deviceIndex_ = 0;
	public:
		virtual void Recreate(SlangCompiledUnit shader) {};

		vk::Pipeline GetPipeline() const
		{
			return pipeline_;
		}
	};
}


