#include "vulkan_rendering.h"
#ifdef ERMY_GAPI_VULKAN
#include "vk_utils.h"

using namespace ermy;
using namespace ermy::rendering;

std::vector<VkPipeline> gAllPipelines;
std::vector<VkPipelineLayout> gAllPipelineLayouts;
std::vector<VkShaderModule> gAllShaderModules;

constexpr uint32_t OpEntryPoint = 15;

std::string _extractEntryPointName(const uint32_t* spirv, int numOpCodes) {
	// Skip the SPIR-V header (5 words)
	size_t index = 5;

	while (index < numOpCodes) {
		uint32_t word = spirv[index];
		uint32_t opcode = word & 0xFFFF; // Lower 16 bits contain the opcode
		uint32_t wordCount = (word >> 16) & 0xFFFF; // Upper 16 bits contain the word count

		if (opcode == OpEntryPoint) {
			// OpEntryPoint instruction format:
			// - Word 0: Opcode and word count
			// - Word 1: Execution model
			// - Word 2: Entry point ID
			// - Word 3+: Entry point name (null-terminated string)

			// The entry point name starts at word 3
			const char* entryPointName = reinterpret_cast<const char*>(&spirv[index + 3]);
			return std::string(entryPointName);
		}

		// Move to the next instruction
		index += wordCount;
	}

	// If no entry point is found, return an empty string
	return "";
}

void _addShader(const ermy::ShaderBytecode& bc, std::vector<VkPipelineShaderStageCreateInfo>& stages)
{
	if (!bc.cachedDeviceObjectID.isValid())
	{
		bc.cachedDeviceObjectID.handle = gAllShaderModules.size();

		VkShaderModuleCreateInfo cinfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		cinfo.pCode = (const uint32_t*)bc.data;
		cinfo.codeSize = bc.size;

		VkShaderModule& shaderModule = gAllShaderModules.emplace_back();
		VK_CALL(vkCreateShaderModule(gVKDevice, &cinfo, gVKGlobalAllocationsCallbacks, &shaderModule));	

		//auto entryPoint = _extractEntryPointName((const uint32_t*)bc.data, bc.size / 4);
		//gAllShaderModulesEntryPoints.push_back(entryPoint);
	}

	auto& newStage = stages.emplace_back();
	newStage.module = gAllShaderModules[bc.cachedDeviceObjectID.handle];
	newStage.stage = vk_utils::VkShaderStageFromErmy(bc.stage);
	newStage.pName = "main";
	newStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
}

VkPipeline _createPipeline(const PSODesc& desc)
{
	VkPipeline result = VK_NULL_HANDLE;

	VkGraphicsPipelineCreateInfo cinfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	VkPipelineLayout layout = VK_NULL_HANDLE;
	{
		VkPipelineLayoutCreateInfo cinfo{};
		cinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkPushConstantRange range;
		range.size = 128;
		range.offset = 0;
		range.stageFlags = VK_SHADER_STAGE_ALL;

		cinfo.pushConstantRangeCount = 1;
		cinfo.pPushConstantRanges = &range;

		vkCreatePipelineLayout(gVKDevice, &cinfo, gVKGlobalAllocationsCallbacks, &layout);
	}

	cinfo.layout = layout;
	gAllPipelineLayouts.push_back(layout);

	VkPipelineVertexInputStateCreateInfo pipVertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	VkPipelineInputAssemblyStateCreateInfo pipInputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	VkPipelineTessellationStateCreateInfo pipTessellationState = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
	VkPipelineViewportStateCreateInfo pipViewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	VkPipelineRasterizationStateCreateInfo pipRasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	VkPipelineMultisampleStateCreateInfo pipMultisamplingState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	VkPipelineDepthStencilStateCreateInfo pipDepthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	VkPipelineColorBlendStateCreateInfo pipColorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	VkPipelineDynamicStateCreateInfo pipDynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

	VkPipelineColorBlendAttachmentState bstate = {};

	bstate.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR;
	bstate.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR;
	bstate.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
	bstate.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA;

	bstate.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
	bstate.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
	bstate.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT;

	bstate.blendEnable = false;
	bstate.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
	bstate.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
	bstate.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
	bstate.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;

	pipInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	pipColorBlendState.attachmentCount = 1;
	pipColorBlendState.pAttachments = &bstate;
	pipColorBlendState.logicOp = VK_LOGIC_OP_COPY;

	std::vector<VkPipelineShaderStageCreateInfo> allStages;
	for (auto& d : desc.shaders)
	{
		_addShader(d, allStages);
	}
	//FillShaderStageIfNeeded(allStages, VK_SHADER_STAGE_VERTEX_BIT, desc.vertexShader);
	//FillShaderStageIfNeeded(allStages, VK_SHADER_STAGE_FRAGMENT_BIT, desc.fragmentShader);

	pipRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	pipRasterizationState.cullMode = VK_CULL_MODE_NONE; //TODO: temporal fix
	pipRasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipRasterizationState.lineWidth = 1.0f;

	VkRect2D viewrect;
	viewrect.extent.width = 1280;
	viewrect.extent.height = 720;
	viewrect.offset = { 0,0 };

	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)1280;
	viewport.height = (float)720;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	pipViewportState.viewportCount = 1;
	pipViewportState.pViewports = &viewport;
	pipViewportState.scissorCount = 1;
	pipViewportState.pScissors = &viewrect;


	pipMultisamplingState.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkDynamicState> all_dinamic_states;

	all_dinamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	all_dinamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);


	pipDepthStencilState.depthWriteEnable = false;// desc.writeDepth;
	pipDepthStencilState.depthTestEnable = false;// desc.writeDepth;
	pipDepthStencilState.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS;

	pipDynamicState.pDynamicStates = all_dinamic_states.data();
	pipDynamicState.dynamicStateCount = (u32)all_dinamic_states.size();

	cinfo.pStages = allStages.data();
	cinfo.stageCount = (u32)allStages.size();

	cinfo.pRasterizationState = &pipRasterizationState;
	cinfo.pColorBlendState = &pipColorBlendState;
	cinfo.pViewportState = &pipViewportState;
	cinfo.pDepthStencilState = &pipDepthStencilState;
	cinfo.pMultisampleState = &pipMultisamplingState;
	cinfo.pVertexInputState = &pipVertexInputState;
	cinfo.pInputAssemblyState = &pipInputAssemblyState;
	cinfo.pDynamicState = &pipDynamicState;

	cinfo.renderPass = gVKRenderPass; //final pass. TODO:// framegraph passes

	VkPipeline pipeline = nullptr;
	VK_CALL(vkCreateGraphicsPipelines(gVKDevice, nullptr /*TODO: PipelineCache*/, 1, &cinfo, gVKGlobalAllocationsCallbacks, &pipeline));
	return pipeline;
}

PSOID ermy::rendering::CreatePSO(const PSODesc& desc)
{
	PSOID id { gAllPipelines.size() };

	gAllPipelines.push_back(_createPipeline(desc));
	return id;
}

#endif