#pragma once

//#include "System/System.h"
#include "GpuInterface/GpuInterface.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "VkBootstrap.h"


#include "Types/Device.h"
#include "Types/Buffer.h"
#include "Types/Texture.h"
#include "Types/Sampler.h"
#include "Types/PipelineGraphics.h"
#include "Types/Swapchain.h"

#include "SDL3/SDL.h"

namespace KE
{
	class GPUI_DLL_API VkGpuInterface : public IGpuInterface
	{
	
		//KE_SYSTEM(VkRenderer);
	public:
		virtual ~VkGpuInterface() = default;

		virtual void Init() override;

		//Dangerzone! SDL does some macro fuckery around the flags. u64 might not work as a type in the future.
		virtual u64 GetSDLWindowFlag() override;


	public:
		vkb::Instance vkboot_inst_;
		vk::Instance vk_inst_;
		
		SDL_Window* window_ = nullptr;
		vk::Queue graphics_queue_;
		vk::Queue compute_queue_;

		vk::CommandPool computeCmdPool;


		VK::Texture testTexture;
		VK::Sampler testSampler;
		VK::Buffer testBuff;
		VK::Buffer vertexBuffer;
		VK::PipelineGraphics testPipeline = VK::PipelineGraphics();
		VK::Swapchain swapchain_;
	};
}