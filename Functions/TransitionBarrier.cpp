#include "TransitionBarrier.h"


void transitionBarrier(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::ImageMemoryBarrier2 barrier{};
    barrier.srcStageMask     = vk::PipelineStageFlagBits2::eTopOfPipe;
    barrier.srcAccessMask    = vk::AccessFlagBits2::eNone;
    barrier.dstStageMask     = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.dstAccessMask    = vk::AccessFlagBits2::eColorAttachmentWrite;
    barrier.oldLayout        = oldLayout;
    barrier.newLayout        = newLayout;
    barrier.image            = image;
    barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

    vk::DependencyInfo depInfo{};
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers    = &barrier;

    cmd.pipelineBarrier2(depInfo);
}

void transitionBarrier(vk::CommandBuffer cmd, KE::VK::SwapchainOutput swapchainOutput, vk::ImageLayout newLayout)
{
    vk::ImageMemoryBarrier2 barrier{};
    barrier.srcStageMask     = vk::PipelineStageFlagBits2::eTopOfPipe;
    barrier.srcAccessMask    = vk::AccessFlagBits2::eNone;
    barrier.dstStageMask     = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.dstAccessMask    = vk::AccessFlagBits2::eColorAttachmentWrite;

    barrier.oldLayout        = swapchainOutput.GetLayout();
    barrier.newLayout        = newLayout;
    barrier.image            = swapchainOutput.GetImage();
    barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    
    vk::DependencyInfo depInfo{};
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers    = &barrier;

    cmd.pipelineBarrier2(depInfo);
}