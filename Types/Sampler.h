#pragma once

#include "Resource.h"

#include "vulkan/vulkan.hpp"
#include "GpuInterfaceDLL.h"

namespace KE::VK
{
class GPUI_DLL_API Sampler : public IResource
{
  friend class DescriptorHandle;
  KE_GPUREFLECT(Sampler)
  protected:
    

    vk::SamplerCreateInfo samplerInfo_;
    u32                   deviceIndex_;
    u32                   samplerHeapIndex_;

  public:
    Sampler() = default;
    Sampler(u32 deviceIndex, bool addToHeap = false);
};
} // namespace KE::VK