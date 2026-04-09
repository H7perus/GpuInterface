#pragma once

#include "GpuInterface/Types/Sampler.h"
#include "GpuInterface/Types/Texture.h"
#include "GpuInterface/Types/Buffer.h"

#include "GpuInterface/BasicTypeAliases.h"

namespace KE::VK
{
    //Represents a slang descriptor handle type. Both textures and buffers will contain a resource index, while textures also contain a sampler index.

    class DescriptorHandle
    {
        u32 resourceIndex_ = 0;
        u32 samplerIndex_ = 0;
    public:
        DescriptorHandle(u32 resourceIndex, u32 samplerIndex)
            : resourceIndex_(resourceIndex), samplerIndex_(samplerIndex) {}

        DescriptorHandle(KE::VK::Buffer buffer) : resourceIndex_(buffer.resourceHeapIndex_) {}
        
        DescriptorHandle(KE::VK::Texture texture, KE::VK::Sampler sampler) : resourceIndex_(texture.resourceHeapIndex_), samplerIndex_(sampler.samplerHeapIndex_) {}


        
    };

}