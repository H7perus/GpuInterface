
#include "Sampler.h"
#include "DeviceManager.h" 


KE::VK::Sampler::Sampler(u32 deviceIndex, bool addToHeap) : deviceIndex_(deviceIndex)
{
    samplerInfo_.magFilter        = vk::Filter::eNearest;
    samplerInfo_.minFilter        = vk::Filter::eNearest;
    samplerInfo_.mipmapMode       = vk::SamplerMipmapMode::eLinear;
    samplerInfo_.addressModeU     = vk::SamplerAddressMode::eRepeat;
    samplerInfo_.addressModeV     = vk::SamplerAddressMode::eRepeat;
    samplerInfo_.addressModeW     = vk::SamplerAddressMode::eRepeat;
    samplerInfo_.mipLodBias       = 0.0f;
    samplerInfo_.anisotropyEnable = vk::False;
    samplerInfo_.minLod           = 0.0f;
    samplerInfo_.maxLod           = vk::LodClampNone;
    samplerInfo_.borderColor      = vk::BorderColor::eIntOpaqueBlack;

    if (addToHeap)
    {
        samplerHeapIndex_ = ContextManager::GetDevice(deviceIndex).EnterSamplerDescriptor(&samplerInfo_);
    }
    else
    {
        throw std::runtime_error("Not implemented!");
    }
}
