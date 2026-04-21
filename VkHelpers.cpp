#include "VkHelpers.h"


void GPUI_DLL_API InitDispatcherForDLL(vk::Instance instance)
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}
