#pragma once
#include <iostream>
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"

class VulkanState {
public:
    vk::Instance instance;
    vk::SurfaceKHR surface;

    vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
    vk::Device device;

    vkb::Instance instanceVKB;

    vkb::Device deviceVKB;
    vkb::Swapchain swapchainVKB;

    vk::CommandPool commandPool;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;



    inline static VulkanState* stateInstance = nullptr;

    static VulkanState& getInstance() {
        if (stateInstance == nullptr) {
            stateInstance = new VulkanState();
        }
        return *stateInstance;
    }

private:
    VulkanState() = default;
    ~VulkanState() = default;
    VulkanState(const VulkanState&) = delete;
    VulkanState& operator=(const VulkanState&) = delete;
};