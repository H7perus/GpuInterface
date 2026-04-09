#pragma once
#include "vulkan/vulkan.hpp"
#include "Buffer.h"
#include "GpuInterface/BasicTypeAliases.h"

#include "GpuInterface/GpuInterface.h"

#include "GpuInterfaceDLL.h"

namespace KE::VK
{
	enum DescriptorHeapType
	{
		SamplerHeap,
		ResourceHeap
	};



	class GPUI_DLL_API DescriptorHeapBuffer : public Buffer
	{
		//lowest index: heap reserved range divided by descriptor size
		i32 lowestIndex_ = 0;
		//the limit. We can only have that many resources referenced by this heap
		i32 maxDescriptorCount_ = 0;
		i32 descriptorSize_ = 0;
		//counts up when a new slot in the heap is used. If an old one is reused, it stays the same.
		//value includes the reserved range
		i32 previouslyUsedIndexCount_ = 0;
		//We want to reuse free descriptor slots. Slot is freed = goes into the list, is reused and the element popped.
		std::vector<i32> freedIndices_;
		vk::BindHeapInfoEXT bindInfo_;
		DescriptorHeapType heapType_;

	public:
		DescriptorHeapBuffer(){};
		DescriptorHeapBuffer( const u32 deviceIndex, i32 descriptorCount, DescriptorHeapType heapType);

		i32 EnterResourceDescriptor(vk::ResourceDescriptorInfoEXT* descriptor);

		i32 EnterSamplerDescriptor(vk::SamplerCreateInfo* sampler);

        vk::BindHeapInfoEXT* GetBindInfo();

      private:
		static i64 calculateSize(const u32 deviceIndex, i64 count, DescriptorHeapType type);
	};
}


