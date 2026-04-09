#include "Texture.h"

#include "DeviceManager.h"


KE::VK::Texture::Texture(u32 deviceIndex, uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage,
                         vk::MemoryPropertyFlags properties, bool sendToHeap)
{
    deviceIndex_ = deviceIndex;

    auto& device = ContextManager::GetDevice(deviceIndex);

    imageInfo_.imageType   = vk::ImageType::e2D;
    imageInfo_.extent      = vk::Extent3D(width, height, 1);
    imageInfo_.format      = format;
    imageInfo_.usage       = usage  | vk::ImageUsageFlagBits::eTransferDst;
    imageInfo_.tiling      = vk::ImageTiling::eOptimal;
    imageInfo_.mipLevels   = 1;
    imageInfo_.arrayLayers = 1;
    imageInfo_.samples     = vk::SampleCountFlagBits::e1;

    image_ = ContextManager::GetDevice(deviceIndex).GetVkDevice().createImage(imageInfo_);
    vk::MemoryRequirements memRequirements = device.GetVkDevice().getImageMemoryRequirements(image_);
    // Allocate
    vk::MemoryAllocateInfo allocInfo{
        memRequirements.size,
        device.FindMemoryType(memRequirements.memoryTypeBits, properties) // properties param from constructor
    };
    memory_ = device.GetVkDevice().allocateMemory(allocInfo);
    device.GetVkDevice().bindImageMemory(image_, memory_, 0);





    viewInfo_ = vk::ImageViewCreateInfo{{},
                                        image_,
                                        vk::ImageViewType::e2D,
                                        format,
                                        {}, // component mapping
                                        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
    

    imageDescriptorInfo_.pView  = &viewInfo_;
    imageDescriptorInfo_.layout = vk::ImageLayout::eShaderReadOnlyOptimal;

    if (sendToHeap)
    {
        vk::ResourceDescriptorInfoEXT descriptor = {};
        descriptor.data.pImage                   = &imageDescriptorInfo_;
        descriptor.type                          = vk::DescriptorType::eSampledImage;
        resourceHeapIndex_                       = device.EnterResourceDescriptor(&descriptor);
    }
    else
    {
        imageView_ = device.GetVkDevice().createImageView(viewInfo_);
    }
}

void KE::VK::Texture::UploadPixels(const void* pixelData)
{

    auto& device   = ContextManager::GetDevice(deviceIndex_);
    auto  vkDevice = device.GetVkDevice();
    // 1. Create staging buffer

    u32 size = imageInfo_.extent.width * imageInfo_.extent.height * imageInfo_.extent.depth * 4;

    vk::BufferCreateInfo stagingInfo{};
    stagingInfo.size        = size;
    stagingInfo.usage       = vk::BufferUsageFlagBits::eTransferSrc;
    stagingInfo.sharingMode = vk::SharingMode::eExclusive;

    vk::Buffer staging = vkDevice.createBuffer(stagingInfo);

    // 2. Allocate + bind staging memory (host visible)
    vk::MemoryRequirements memReqs = vkDevice.getBufferMemoryRequirements(staging);
    vk::MemoryAllocateInfo allocInfo{
        memReqs.size, device.FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible |
                                                                        vk::MemoryPropertyFlagBits::eHostCoherent)};
    vk::DeviceMemory stagingMemory = vkDevice.allocateMemory(allocInfo);
    vkDevice.bindBufferMemory(staging, stagingMemory, 0);

    // 3. Copy data in
    void* mapped = vkDevice.mapMemory(stagingMemory, 0, size);
    memcpy(mapped, pixelData, size);
    vkDevice.unmapMemory(stagingMemory);

    vk::CommandPool cmdPool = device.CreateGraphicsCommandPool();

    vk::CommandBufferAllocateInfo cmdAllocInfo{cmdPool, vk::CommandBufferLevel::ePrimary, 1};
    vk::CommandBuffer             cmd = vkDevice.allocateCommandBuffers(cmdAllocInfo).front();

    cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    // 4a. Transition image to transfer dst
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout        = vk::ImageLayout::eUndefined;
    barrier.newLayout        = vk::ImageLayout::eTransferDstOptimal;
    barrier.srcAccessMask    = {};
    barrier.dstAccessMask    = vk::AccessFlagBits::eTransferWrite;
    barrier.image            = image_;
    barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {},
                        barrier);

    // 4b. Copy buffer to image
    vk::BufferImageCopy region{};
    region.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    region.imageExtent      = imageInfo_.extent;

    cmd.copyBufferToImage(staging, image_, vk::ImageLayout::eTransferDstOptimal, region);

    // 4c. Transition to shader read
    barrier.oldLayout     = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                        barrier);

    cmd.end();

    // 5. Submit and wait
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmd;

    auto queue = ContextManager::GetDevice(deviceIndex_).GetGraphicsQueue();
    queue.submit(submitInfo);
    queue.waitIdle(); // naive sync, fine for now

    // 6. Cleanup
    vkDevice.freeCommandBuffers(cmdPool, cmd);
    vkDevice.destroyCommandPool(cmdPool);
    vkDevice.destroyBuffer(staging);
    vkDevice.freeMemory(stagingMemory);
}
