#pragma once
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"


inline uint32_t findMemoryType(vkb::Device device, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {

	vk::PhysicalDevice physicalDevice = vk::PhysicalDevice(device.physical_device);
	vk::PhysicalDeviceMemoryProperties memProperties;

	physicalDevice.getMemoryProperties(&memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

//vk::CommandBuffer beginSingleTimeCommands() {
//	vk::CommandBufferAllocateInfo allocInfo;
//	allocInfo.level = vk::CommandBufferLevel(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
//	allocInfo.commandPool = VulkanState::getInstance().commandPool;
//	allocInfo.commandBufferCount = 1;
//
//	vk::CommandBuffer commandBuffer;
//	VulkanState::getInstance().device.allocateCommandBuffers(&allocInfo, &commandBuffer);
//
//
//	vk::CommandBufferBeginInfo beginInfo;
//	beginInfo.flags = vk::CommandBufferUsageFlags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
//
//	commandBuffer.begin(beginInfo);
//
//	return commandBuffer;
//}

//void endSingleTimeCommands(vk::CommandBuffer& commandBuffer) {
//	commandBuffer.end();
//
//	vk::SubmitInfo submitInfo;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	VulkanState::getInstance().graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
//
//	VulkanState::getInstance().graphicsQueue.waitIdle();
//
//	VulkanState::getInstance().device.freeCommandBuffers(VulkanState::getInstance().commandPool, commandBuffer);
//}

//void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
//	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
//	vk::ImageMemoryBarrier barrier;
//	barrier.oldLayout = oldLayout;
//	barrier.newLayout = newLayout;
//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	barrier.image = image;
//	barrier.subresourceRange.aspectMask = vk::ImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT);
//	barrier.subresourceRange.baseMipLevel = 0;
//	barrier.subresourceRange.levelCount = 1;
//	barrier.subresourceRange.baseArrayLayer = 0;
//	barrier.subresourceRange.layerCount = 1;
//
//	vk::PipelineStageFlags sourceStage;
//	vk::PipelineStageFlags destinationStage;
//
//	if (oldLayout == vk::ImageLayout(VK_IMAGE_LAYOUT_UNDEFINED) && newLayout == vk::ImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
//		barrier.srcAccessMask = vk::AccessFlags(0);
//		barrier.dstAccessMask = vk::AccessFlags(VK_ACCESS_TRANSFER_WRITE_BIT);
//
//		sourceStage = vk::PipelineStageFlags(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
//		destinationStage = vk::PipelineStageFlags(VK_PIPELINE_STAGE_TRANSFER_BIT);
//	}
//	else if (oldLayout == vk::ImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && newLayout == vk::ImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
//		barrier.srcAccessMask = vk::AccessFlags(VK_ACCESS_TRANSFER_WRITE_BIT);
//		barrier.dstAccessMask = vk::AccessFlags(VK_ACCESS_SHADER_READ_BIT);
//
//		sourceStage = vk::PipelineStageFlags(VK_PIPELINE_STAGE_TRANSFER_BIT);
//		destinationStage = vk::PipelineStageFlags(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
//	}
//	else {
//		throw std::invalid_argument("unsupported layout transition!");
//	}
//
//
//
//	commandBuffer.pipelineBarrier(sourceStage,
//		destinationStage,
//		vk::DependencyFlags(),
//		0, nullptr,
//		0, nullptr,
//		1, &barrier
//	);
//	endSingleTimeCommands(commandBuffer);
//}

//void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
//	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
//
//	vk::BufferImageCopy region;
//	region.bufferOffset = 0;
//	region.bufferRowLength = 0;
//	region.bufferImageHeight = 0;
//
//	region.imageSubresource.aspectMask = vk::ImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT);
//	region.imageSubresource.mipLevel = 0;
//	region.imageSubresource.baseArrayLayer = 0;
//	region.imageSubresource.layerCount = 1;
//
//	region.imageOffset = vk::Offset3D{ 0, 0, 0 };
//	region.imageExtent = vk::Extent3D{
//		width,
//		height,
//		1
//	};
//	commandBuffer.copyBufferToImage(vk::Buffer(buffer), vk::Image(image), vk::ImageLayout::eTransferDstOptimal, 1, &region);
//
//	endSingleTimeCommands(commandBuffer);
//}