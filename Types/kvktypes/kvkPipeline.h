#pragma once
#include <vulkan/vulkan.hpp>
#include "memHelper.h"
#include "kvkBuffer.h"
#include "VulkanState.h"

/*
I need to think about exactly what I need:

1. One pipeline is required per shader permutation. Multiple materials with the same shader permutation use the same pipeline
2. One shader permutation might have multiple pipelines (see Z-Prepass)

3. The pipeline should store each object that is unique to the material permutation. That might include vertex input information and descriptor set layouts

4. Pipelines should be easily recreatable if one (or more) of the non-unique parameters changes. That might include enabling wireframe mode or changing the sample count (though that can also be done dynamically via dynamic state)

What follows:
1. We must store the Pipeline
2. We must store the descriptorsetlayout and vertexinputinformation


*/

namespace kvk {

	//TODO: Look into pipeline caching eventually!
	struct RenderPipeline //for now assuming 2D, RGBA
	{
		vk::Pipeline pipeline;

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo; //input datastream. Not sure if that changes with task and mesh shaders
		vk::DescriptorSetLayout descriptorSetLayout; //This informs on buffer and sampler-binding and more

		vk::PipelineLayout pipelineLayout;


		void addShaderStage(vk::ShaderStageFlagBits flags, vk::ShaderModule shaderModule, const char* name)
		{

			vk::PipelineShaderStageCreateInfo shaderStageInfo;

			shaderStageInfo.stage = flags;
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = name;

			shaderStages.push_back(shaderStageInfo);
		}


		void create()
		{
			VulkanState& VKS = VulkanState::getInstance();

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			vk::PipelineViewportStateCreateInfo viewportState{};
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			vk::PipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = vk::PolygonMode::eFill;

			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = vk::CullModeFlagBits::eBack;
			rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

			vk::PipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional

			vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = vk::ColorComponentFlags(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor(VK_BLEND_FACTOR_ONE); // Optional


			colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor(VK_BLEND_FACTOR_ZERO); // Optional
			colorBlendAttachment.colorBlendOp = vk::BlendOp(VK_BLEND_OP_ADD); // Optional
			colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor(VK_BLEND_FACTOR_ONE); // Optional
			colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor(VK_BLEND_FACTOR_ZERO); // Optional
			colorBlendAttachment.alphaBlendOp = vk::BlendOp(VK_BLEND_OP_ADD); // Optional

			vk::PipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = vk::LogicOp(VK_LOGIC_OP_COPY); // Optional
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

			pipelineLayout = VKS.device.createPipelineLayout(pipelineLayoutInfo);

			std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eRasterizationSamplesEXT,
			//vk::DynamicState::eSampleMaskEXT,
			//vk::DynamicState::eAlphaToCoverageEnableEXT,
			vk::DynamicState::eViewport, //can also be eViewportWithCount, if you want the number of viewports to be dynamic as well, I am not really seeing how we might need more than one so far
			vk::DynamicState::eScissor
			};
			vk::PipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			vk::PipelineRenderingCreateInfo PipeRenderCreateInfo;
			PipeRenderCreateInfo.colorAttachmentCount = 1;
			PipeRenderCreateInfo.pColorAttachmentFormats = (vk::Format*)(&VKS.swapchainVKB.image_format);

			PipeRenderCreateInfo.depthAttachmentFormat = vk::Format(VK_FORMAT_D32_SFLOAT);
			PipeRenderCreateInfo.stencilAttachmentFormat = vk::Format(VK_FORMAT_UNDEFINED);
			PipeRenderCreateInfo.viewMask = 0;

			vk::PipelineDepthStencilStateCreateInfo DepthStencilCreateInfo;
			DepthStencilCreateInfo.depthCompareOp = vk::CompareOp::eGreaterOrEqual;
			DepthStencilCreateInfo.depthTestEnable = VK_TRUE;
			DepthStencilCreateInfo.depthWriteEnable = VK_TRUE;

			vk::GraphicsPipelineCreateInfo pipelineInfo;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pNext = &PipeRenderCreateInfo;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &DepthStencilCreateInfo; // Optional
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;


			pipeline = VKS.device.createGraphicsPipeline({}, pipelineInfo, nullptr).value;
		}

		void destroy()
		{
			VulkanState& VKS = VulkanState::getInstance();

			VKS.device.destroyPipeline(pipeline, nullptr);
			VKS.device.destroyPipelineLayout(pipelineLayout, nullptr);
		}
	};
}

