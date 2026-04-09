#include "SubmitAndPresent.h"
#include "../DeviceManager.h"



KE::VK::SubmitAndPresent::SubmitAndPresent(u32 swapchainIndex) : swapchainIndex_(swapchainIndex)
{
    Reads("SwapchainFrame");
    Reads("SwapchainOutput");
}

void KE::VK::SubmitAndPresent::Execute(vk::CommandBuffer cmd)
{
    ContextManager::GetSwapchain(swapchainIndex_).EndFrame();
}
