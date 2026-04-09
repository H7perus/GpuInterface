#pragma once

#include "Resource.h"

#include "vulkan/vulkan.hpp"
#include "GpuInterfaceDLL.h"

namespace KE::VK
{

// This exists so that we can treat swapchain output as its own resource for the use in nodes, such as the present node
// and anything that renders to it.

class GPUI_DLL_API SwapchainOutput : public IResource
{
    KE_GPUREFLECT(SwapchainOutput)
    
    vk::Image       image_;
    vk::ImageView   imageView_;
    vk::Extent2D    extent_;
    vk::ImageLayout layout_;

  public:
    SwapchainOutput() = default;
    SwapchainOutput(vk::Image image, vk::ImageView imageView, vk::Extent2D extent, vk::ImageLayout layout)
        : image_(image), imageView_(imageView), extent_(extent), layout_(layout)
    {
    }
    ~SwapchainOutput() = default;

    vk::Image GetImage() const
    {
        return image_;
    }
    vk::ImageView GetImageView() const
    {
        return imageView_;
    }
    vk::ImageLayout GetLayout() const
    {
        return layout_;
    }

    vk::Extent2D GetExtent() const
    {
        return extent_;
    }
};
} // namespace KE::VK