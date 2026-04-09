#include "vulkan/vulkan.hpp"


struct DescriptorSetLayoutInfo {
    vk::DescriptorSetLayout layout;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};