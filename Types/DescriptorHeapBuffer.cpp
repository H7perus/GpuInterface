#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "DescriptorHeapBuffer.h"

#include "DeviceManager.h"
using namespace KE::VK;

DescriptorHeapBuffer::DescriptorHeapBuffer(const u32 deviceIndex, i32 descriptorCount, DescriptorHeapType heapType)
    : Buffer(deviceIndex, calculateSize(deviceIndex, descriptorCount, heapType),
             vk::BufferUsageFlagBits::eDescriptorHeapEXT | vk::BufferUsageFlagBits::eShaderDeviceAddress,
             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
{
    heapType_         = heapType;
    Device& device_in = ContextManager::GetDevice(deviceIndex);
    if (heapType == ResourceHeap)
    {
        descriptorSize_ = device_in.GetResourceDescriptorStride();
        lowestIndex_    = device_in.GetResourceHeapReservedRange() / device_in.GetResourceDescriptorStride();
    }
    else if (heapType == SamplerHeap)
    {
        descriptorSize_ = device_in.GetSamplerDescriptorSize();
        lowestIndex_    = device_in.GetSamplerHeapReservedRange() / device_in.GetSamplerDescriptorSize();
    }
    else
    {
        throw new std::exception(
            "There are only two types of descriptor heaps and you doofus somehow managed to fuck it up");
    }

    previouslyUsedIndexCount_ = lowestIndex_;
    maxDescriptorCount_       = descriptorCount;

    bindInfo_.heapRange.address   = ((vk::Device)(device_in)).getBufferAddress(buffer);
    bindInfo_.heapRange.size      = bufferCreateInfo.size;
    bindInfo_.reservedRangeOffset = 0;
    bindInfo_.reservedRangeSize   = lowestIndex_ * descriptorSize_;
}

i32 DescriptorHeapBuffer::EnterResourceDescriptor(vk::ResourceDescriptorInfoEXT* descriptor)
{
    assert(heapType_ == ResourceHeap && "Descriptor Heap is not a resource heap!");
    Device& device = ContextManager::GetDevice(deviceIndex_);
    i32     index  = 0;
    if (freedIndices_.size() == 0)
    {
        index = previouslyUsedIndexCount_;
        previouslyUsedIndexCount_++;
    }
    else
    {
        index = freedIndices_[freedIndices_.size() - 1];
        freedIndices_.pop_back();
    }

    vk::HostAddressRangeEXT dest;
    dest.address = (char*)map() + index * descriptorSize_;
    dest.size    = descriptorSize_;


    std::cout << "BEFORE: " << std::endl;
    for(int i = 0; i < 64; i++)
        std::cout << std::dec << (i32)((u8*)dest.address)[i] << std::endl;

    auto result = ((vk::Device)(device)).writeResourceDescriptorsEXT(1, descriptor, &dest);


    std::cout << "AFTER: " << std::endl;
    for(int i = 0; i < 64; i++)
        std::cout << std::dec << (i32)((u8*)dest.address)[i] << std::endl;

    unmap();
    return index;
}

i32 DescriptorHeapBuffer::EnterSamplerDescriptor(vk::SamplerCreateInfo* sampler)
{
    assert(heapType_ == SamplerHeap && "Descriptor Heap is not a sampler heap!");
    Device& device = ContextManager::GetDevice(deviceIndex_);

    i32 index = 0;
    if (freedIndices_.size() == 0)
    {
        index = previouslyUsedIndexCount_;
    }
    else
    {
        index = freedIndices_[freedIndices_.size() - 1];
        freedIndices_.pop_back();
    }

    vk::HostAddressRangeEXT dest;
    dest.address = (char*)map() + index * descriptorSize_;
    dest.size    = descriptorSize_;

    auto result = ((vk::Device)(device)).writeSamplerDescriptorsEXT(1, sampler, &dest);

    for(int i = 0; i < 64; i++)
        std::cout << std::dec << (i32)((u8*)dest.address)[i] << std::endl;

    unmap();
    return index;
}

vk::BindHeapInfoEXT* KE::VK::DescriptorHeapBuffer::GetBindInfo()
{
    return &bindInfo_;
};


i64 DescriptorHeapBuffer::calculateSize(const u32 deviceIndex, i64 count, DescriptorHeapType type)
{
    Device& device_in = ContextManager::GetDevice(deviceIndex);
    if (type == ResourceHeap)
        return device_in.GetResourceHeapReservedRange() + count * device_in.GetResourceDescriptorStride();
    else
        return device_in.GetSamplerHeapReservedRange() + count * device_in.GetSamplerDescriptorSize();
}