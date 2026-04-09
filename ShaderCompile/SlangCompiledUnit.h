#pragma once


#include "vulkan/vulkan.hpp"

#include "slang.h"
#include "slang-com-ptr.h"


#include "../Types/Device.h"


//Intent: Maintain the composed program and provide helpers for reflection etc.

class SlangCompiledUnit
{
public:
	SlangCompiledUnit(Slang::ComPtr <slang::IComponentType> program) : composedProgram(program) {};

	Slang::ComPtr<slang::IComponentType> composedProgram;

	Slang::ComPtr<slang::IBlob> getTargetCode()
	{
		Slang::ComPtr<slang::IBlob> blob;
		composedProgram->getTargetCode(0, blob.writeRef());
		return blob;
	}

	// uses reflection to provide the correct descriptor set layout

	std::pair<vk::DescriptorSetLayout, std::vector<vk::DescriptorSetLayoutBinding>> createDescriptorSetLayout(const vk::Device& device)
	{
		auto layout = composedProgram->getLayout();
		//auto layout = composedProgram->getLayout();
		auto typeLayout = layout->getGlobalParamsTypeLayout();

		const int fieldCount = typeLayout->getFieldCount();

		std::vector<vk::DescriptorSetLayoutBinding> bindings(fieldCount);

		for (int i = 0; i < fieldCount; i++)
		{
			slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(i);

			slang::TypeLayoutReflection* typeLayout = field->getTypeLayout();
			slang::TypeReflection* type = typeLayout->getType();

			bindings[i].binding = field->getBindingIndex();

			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = vk::ShaderStageFlagBits::eAll;
			bindings[i].pImmutableSamplers = nullptr;

			switch (type->getKind())
			{
				case slang::TypeReflection::Kind::ConstantBuffer:
				{
					bindings[i].descriptorType = vk::DescriptorType::eUniformBuffer;
					break;
				}
				case slang::TypeReflection::Kind::Resource:
				{
					SlangResourceShape shape = type->getResourceShape();
					//unused as of now. Vulkan dgaf
					SlangResourceAccess access = type->getResourceAccess();

					if (shape == SLANG_STRUCTURED_BUFFER) {
						bindings[i].descriptorType = vk::DescriptorType::eStorageBuffer;
					}
					else if (shape & SLANG_TEXTURE_COMBINED_FLAG)
					{
						bindings[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
					}
					break;
				}
			}
		}

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		return std::pair(device.createDescriptorSetLayout(layoutInfo), bindings);
	}


};