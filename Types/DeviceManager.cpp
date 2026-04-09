#include "DeviceManager.h"
using namespace KE::VK;

void KE::VK::ContextManager::Init()
{
    GetInstance();
}
KE::VK::ContextManager &KE::VK::ContextManager::GetInstance()
{
    static ContextManager instance;
    return instance;
};

void KE::VK::ContextManager::SetVkInstance(vkb::Instance inst)
{
    GetInstance().vkboot_inst_ = inst;
}

vk::Instance KE::VK::ContextManager::GetVkInstance()
{
    return GetInstance().vkboot_inst_.instance;
}

Device& ContextManager::GetDevice(u32 index)
{
    return GetInstance().devices_.at(index);
}

Swapchain& ContextManager::GetSwapchain(u32 index)
{
    if(GetInstance().swapchains_.find(index) == GetInstance().swapchains_.end())
    {
        throw std::runtime_error("Swapchain index not found");
    }
    return GetInstance().swapchains_.at(index);
}


