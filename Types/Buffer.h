#pragma once


#include <vulkan/vulkan.hpp>

#include "GpuInterface/BasicTypeAliases.h"
#include "Resource.h"

#include "GpuInterfaceDLL.h"

namespace KE::VK
{
struct GPUI_DLL_API Buffer : public IResource
{
    KE_GPUREFLECT(Buffer)
    friend class DescriptorHandle;
  protected:
    vk::Buffer       buffer = VK_NULL_HANDLE;
    vk::DeviceMemory memory = VK_NULL_HANDLE;
    vk::DeviceAddressRangeEXT addressRange_;
    vk::BufferCreateInfo bufferCreateInfo{};

    u32  deviceIndex_ = 0;
    u32  resourceHeapIndex_;
    bool isMappable = false;

    

  public:
    ~Buffer();

    Buffer() = default;
    Buffer(u32 deviceIndex, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, bool sendToHeap = false);

    Buffer(Buffer&& other);

    Buffer& operator=(Buffer&& other) noexcept;

    vk::DescriptorBufferInfo GetDescriptorBufferInfo(u32 offset, u64 range);

  protected:
    vk::DeviceAddress GetBufferAddress();

    

  public:
    vk::DeviceAddressRangeEXT* GetBufferAddressRangePtr();


    vk::Buffer GetVkBuffer() {return buffer;}

    void* map();
    void  unmap();
    void  destroy();
};
} // namespace KE::VK
