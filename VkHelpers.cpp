#include "VkHelpers.h"


void GPUI_DLL_API InitDispatcherForDLL(vk::Instance instance)
{
    //dummy call to init the vkb function table.
    createInstance();

    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}
