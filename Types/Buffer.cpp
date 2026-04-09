#include "Buffer.h"

#include "DeviceManager.h"
#include "vulkan/vulkan.hpp"

using namespace KE::VK;

Buffer::~Buffer()
{

    if (buffer != VK_NULL_HANDLE)
        destroy();
}

Buffer::Buffer(u32 deviceIndex, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
               bool sendToHeap)
    : deviceIndex_(deviceIndex)
{
    Device& device               = ContextManager::GetInstance().GetDevice(deviceIndex_);
    bufferCreateInfo.size        = size;
    bufferCreateInfo.usage       = usage | vk::BufferUsageFlagBits::eShaderDeviceAddress;
    bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    if (((vk::Device)(device)).createBuffer(&bufferCreateInfo, nullptr, &buffer) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    vk::MemoryRequirements memRequirements;
    ((vk::Device)(device)).getBufferMemoryRequirements(buffer, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};

    vk::MemoryAllocateFlagsInfo flagsInfo;
    flagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;

    if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eShaderDeviceAddress)
    {

        allocInfo.pNext = &flagsInfo;
    }

    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        device.FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlags(properties));

    if (((vk::Device)(device)).allocateMemory(&allocInfo, nullptr, &memory) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    if ((properties & vk::MemoryPropertyFlagBits::eHostVisible))
        isMappable = true;

    ((vk::Device)(device)).bindBufferMemory(buffer, memory, 0);

    addressRange_.address = GetBufferAddress();
    addressRange_.size    = bufferCreateInfo.size;


    if (sendToHeap)
    {
        vk::ResourceDescriptorInfoEXT descriptor = {};
        descriptor.data                          = &addressRange_;
        descriptor.type                          = vk::DescriptorType::eStorageBuffer;
        resourceHeapIndex_ = ContextManager::GetDevice(deviceIndex).EnterResourceDescriptor(&descriptor);
    }
}

Buffer::Buffer(Buffer&& other)
    : buffer(other.buffer), memory(other.memory), isMappable(other.isMappable),
      bufferCreateInfo(other.bufferCreateInfo), deviceIndex_(other.deviceIndex_), addressRange_(other.addressRange_)
{
    other.buffer = VK_NULL_HANDLE;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other)
    {
        // Release any resources currently held by this object
        if (buffer)
        {
            destroy();
        }

        // Transfer ownership of the resources of the new object
        deviceIndex_     = other.deviceIndex_;
        addressRange_    = other.addressRange_;
        buffer           = other.buffer;
        memory           = other.memory;
        bufferCreateInfo = other.bufferCreateInfo;
        isMappable       = other.isMappable;
        other.buffer     = VK_NULL_HANDLE;
        other.memory     = VK_NULL_HANDLE;
    }
    return *this;
}

vk::DescriptorBufferInfo Buffer::GetDescriptorBufferInfo(u32 offset, u64 range)
{
    return vk::DescriptorBufferInfo(buffer, offset, range);
}


vk::DeviceAddress Buffer::GetBufferAddress()
{
    return ((vk::Device)(ContextManager::GetInstance().GetDevice(deviceIndex_))).getBufferAddress(buffer);
}

vk::DeviceAddressRangeEXT* KE::VK::Buffer::GetBufferAddressRangePtr()
{
    return &addressRange_;
}


void* KE::VK::Buffer::map()
{
    if (!isMappable)
    {
        std::cerr << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT was not requested for this piece of memory";
        std::terminate();
    }

    Device& device = ContextManager::GetInstance().GetDevice(deviceIndex_);

    void* data;
    data = ((vk::Device)(device)).mapMemory(memory, 0, bufferCreateInfo.size, vk::MemoryMapFlags(0));

    return data;
}
void KE::VK::Buffer::unmap()
{
    ((vk::Device)(ContextManager::GetInstance().GetDevice(deviceIndex_))).unmapMemory(memory);
}
void KE::VK::Buffer::destroy()
{
    if (buffer != VK_NULL_HANDLE)
    {
        ((vk::Device)(ContextManager::GetInstance().GetDevice(deviceIndex_))).destroyBuffer(buffer, nullptr);
        if (memory == VK_NULL_HANDLE)
            throw("literally HOW?");
        ((vk::Device)(ContextManager::GetInstance().GetDevice(deviceIndex_))).freeMemory(memory, nullptr);

        buffer = VK_NULL_HANDLE;
    }
}
