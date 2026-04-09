#pragma once
#include <vulkan/vulkan.hpp>
#include "memHelper.h"
#include "VulkanState.h"

namespace kvk {
	struct Buffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		bool isMappable = false;
		VkBufferCreateInfo bufferInfo{};

		~Buffer()
		{
			if (buffer != VK_NULL_HANDLE)
				destroy();
		}
		Buffer() {}
		Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

			vk::Device& vdevice = VulkanState::getInstance().device;
			VkDevice device = VkDevice(vdevice);

			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlags(properties));

			if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate buffer memory!");
			}
			if ((properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
				isMappable = true;

			vkBindBufferMemory(device, buffer, memory, 0);
		}

		Buffer(Buffer&& other) : buffer(other.buffer), memory(other.memory), isMappable(other.isMappable), bufferInfo(other.bufferInfo)
		{
			other.buffer = VK_NULL_HANDLE;
		}

		Buffer& operator= (Buffer&& other) noexcept
		{
			if (this != &other)
			{
				// Release any resources currently held by this object
				if (buffer)
				{
					destroy();
				}

				// Transfer ownership of the resources of the new object
				buffer = other.buffer;
				memory = other.memory;
				bufferInfo = other.bufferInfo;
				isMappable = other.isMappable;

				other.buffer = VK_NULL_HANDLE;
				other.memory = VK_NULL_HANDLE;
			}
			return *this;
		}



		void* map()
		{
			if (!isMappable)
			{
				std::cerr << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT was not requested for this piece of memory";
				std::terminate();
			}
			vk::Device& vdevice = VulkanState::getInstance().device;
			VkDevice device = VkDevice(vdevice);
			void* data;
			vkMapMemory(device, memory, 0, bufferInfo.size, 0, &data);
			return data;
		}
		void unmap()
		{
			vk::Device& vdevice = VulkanState::getInstance().device;
			VkDevice device = VkDevice(vdevice);
			vkUnmapMemory(device, memory);
		}
		void destroy()
		{
			vk::Device& vdevice = VulkanState::getInstance().device;
			VkDevice device = VkDevice(vdevice);

			if (buffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(device, buffer, nullptr);
				if (memory == VK_NULL_HANDLE)
					throw("literally HOW?");

				vkFreeMemory(device, memory, nullptr);

				buffer = VK_NULL_HANDLE;
			}

		}
	};

}

