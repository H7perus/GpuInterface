
#include "Frame.h"

void KE::VK::InflightFrame::Init(KE::VK::Device &device) 
{
    commandPool_ = device.CreateGraphicsCommandPool();
    commandBuffer_ = device.GetVkDevice().allocateCommandBuffers({commandPool_, vk::CommandBufferLevel::ePrimary, 1})[0];

    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
    inFlightFence_ = device.GetVkDevice().createFence(fenceInfo);
    
    vk::SemaphoreCreateInfo semaphoreInfo{};
    imageAvailableSemaphore_ = device.GetVkDevice().createSemaphore(semaphoreInfo);

    renderFinishedSemaphore_ = device.GetVkDevice().createSemaphore(semaphoreInfo);
}

void KE::VK::InflightFrame::Destroy(KE::VK::Device &device) 
{
    device.GetVkDevice().destroySemaphore(imageAvailableSemaphore_);
    device.GetVkDevice().destroySemaphore(renderFinishedSemaphore_);
    device.GetVkDevice().destroyFence(inFlightFence_);
    device.GetVkDevice().destroyCommandPool(commandPool_);
}


// TODO: This could be a stored part of the frame, no?
vk::SubmitInfo *KE::VK::InflightFrame::GetSubmitInfoPtr() {
  submitInfo_.waitSemaphoreCount = 1;
  submitInfo_.pWaitSemaphores = &imageAvailableSemaphore_;
  submitInfo_.pSignalSemaphores = &renderFinishedSemaphore_;
  submitInfo_.pCommandBuffers = &commandBuffer_;
  return &submitInfo_;
}
