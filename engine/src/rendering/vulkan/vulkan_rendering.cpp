#include "vulkan_rendering.h"
#include "vulkan_swapchain.h"
#ifdef ERMY_GAPI_VULKAN
#include "vk_utils.h"

using namespace ermy;
using namespace ermy::rendering;

std::vector<VkPipeline> gAllPipelines;
std::vector<VkPipelineLayout> gAllPipelineLayouts;
std::vector<VkDescriptorSetLayout> gAllPipelineDSLayouts;
std::vector<VkShaderModule> gAllShaderModules;

std::vector<VkBuffer> gAllBuffers;

std::vector<VkImage> gAllImages;
std::vector<VkImageView> gAllImageViews;
std::vector<VkDescriptorSet> gAllImageDescriptors;
std::vector<ImageMeta> gAllImageMetas;

std::vector<RenderpassInfo> gAllRenderPasses;

struct TextureInfo
{
	VkImage image;
	VkImageView imageView;
	VkDescriptorSet descriptor;
	ImageMeta meta;
};

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
		bc.cachedDeviceObjectID.handle = static_cast<u16>(gAllShaderModules.size());

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

RenderpassInfo _createRenderPass(const RenderPassDesc& desc)
{
	RenderpassInfo result;
	const ImageMeta& meta = gAllImageMetas[desc.colorAttachment.handle];
	VkImage image = gAllImages[desc.colorAttachment.handle];
	VkImageView imageView = gAllImageViews[desc.colorAttachment.handle];

	VkRenderPassCreateInfo rpass = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = meta.format;
	colorAttachment.samples = meta.samples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	//VkSubpassDependency dependency = {};
	//dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // External dependency
	//dependency.dstSubpass = 0; // Our single subpass
	//dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//dependency.srcAccessMask = 0;
	//dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // External dependency (previous render pass)
	dependency.dstSubpass = 0; // The subpass that reads the image
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Stage where the render pass writes
	dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Stage where the shader reads
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Access mask for the render pass
	dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for the shader
	dependency.dependencyFlags = 0;

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //Assuming you're reading the image in the fragment shader. Insert other shader stage(s) if otherwise.
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //renderpass A wrote to the image as an attachment.
	dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; //renderpass B is reading from the image in a shader.
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; //By region dependencies make no sense here, since they're not part of the same renderpass.

	rpass.attachmentCount = 1;
	rpass.pAttachments = &colorAttachment;
	rpass.subpassCount = 1;
	rpass.pSubpasses = &subpass;
	rpass.dependencyCount = 0;
	rpass.pDependencies = &dependency;

	vkCreateRenderPass(gVKDevice, &rpass, gVKGlobalAllocationsCallbacks, &result.renderpass);

	VkFramebufferCreateInfo fbuff = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };

	fbuff.renderPass = result.renderpass; // Associate the framebuffer with the render pass
	fbuff.attachmentCount = 1;
	fbuff.pAttachments = &imageView; // Use the provided VkImageView
	fbuff.width = meta.width; // Use the provided extent
	fbuff.height = meta.height;
	fbuff.layers = 1; // Single layer

	vkCreateFramebuffer(gVKDevice, &fbuff, gVKGlobalAllocationsCallbacks, &result.framebuffer);
	
	result.defaultWidth = meta.width;
	result.defaultHeight = meta.height;
	result.targetImage = image;
	return result;
}

TextureInfo _createTexture(const TextureDesc& desc)
{
	VkDeviceSize imageSize = desc.width * desc.height * 4; // 4 channels (RGBA) TODO: from format
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferAllocation;
	void* pixelsCPUdata  = nullptr; 
	ImageMeta meta;

	VkImageUsageFlags textureUsage = VK_IMAGE_USAGE_SAMPLED_BIT;

	if (desc.pixelsData)
	{
		textureUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else
	{
		textureUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	meta.width = static_cast<uint32_t>(desc.width);
	meta.height = static_cast<uint32_t>(desc.height);
	meta.format = VK_FORMAT_R8G8B8A8_UNORM;
	meta.samples = VK_SAMPLE_COUNT_1_BIT;

	// Create a Vulkan image using VMA
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(desc.width);
	imageInfo.extent.height = static_cast<uint32_t>(desc.height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // 4-channel format
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = textureUsage ;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	
	VmaAllocationCreateInfo imageAllocCreateInfo{};
	imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImage textureImage = VK_NULL_HANDLE;
	VmaAllocation textureImageAllocation = VK_NULL_HANDLE;
	VmaAllocationInfo textureImageAllocationInfo;
	VK_CALL(vmaCreateImage(gVMA_Allocator, &imageInfo, &imageAllocCreateInfo, &textureImage, &textureImageAllocation, &textureImageAllocationInfo));

	if (desc.pixelsData)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		VmaAllocationInfo bufferAllocationInfo;
		VK_CALL(vmaCreateBuffer(gVMA_Allocator, &bufferInfo, &allocCreateInfo, &stagingBuffer, &stagingBufferAllocation, &bufferAllocationInfo));

		vmaMapMemory(gVMA_Allocator, stagingBufferAllocation, &pixelsCPUdata);
		memcpy(pixelsCPUdata, desc.pixelsData, static_cast<size_t>(imageSize));
		vmaUnmapMemory(gVMA_Allocator, stagingBufferAllocation);

		auto c = AllocateSingleTimeCommandBuffer();
		vk_utils::ImageTransition(c.cbuff, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { desc.width, desc.height, 1 };

		vkCmdCopyBufferToImage(c.cbuff, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		vk_utils::ImageTransition(c.cbuff, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		c.Sumbit();
	}

	// Create an image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView textureImageView;
	VK_CALL(vkCreateImageView(gVKDevice, &viewInfo, nullptr, &textureImageView));

	// Create a descriptor set for the texture
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = gStaticDescriptorsPool; // Assume this is created elsewhere
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &gImguiPreviewLayout; // Assume this is created elsewhere

	VkDescriptorSet textureDescriptorSet;
	VK_CALL(vkAllocateDescriptorSets(gVKDevice, &allocInfo, &textureDescriptorSet));

	VkDescriptorImageInfo imageInfoDesc{};
	imageInfoDesc.imageView = textureImageView;
	imageInfoDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfoDesc.sampler = gLinearSampler;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = textureDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfoDesc;

	vkUpdateDescriptorSets(gVKDevice, 1, &descriptorWrite, 0, nullptr);

	TextureInfo result;
	result.image = textureImage;
	result.imageView = textureImageView;
	result.descriptor = textureDescriptorSet;
	result.meta = meta;

	return result;
}

VkBuffer _createBuffer(const BufferDesc& desc)
{
	return VK_NULL_HANDLE;
}

VkPipeline _createPipeline(const PSODesc& desc)
{

	VkPipeline result = VK_NULL_HANDLE;

	VkGraphicsPipelineCreateInfo cinfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;

	{
		VkPipelineLayoutCreateInfo cinfo{};
		cinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkPushConstantRange range;
		
		if (desc.numRootConstants > 0)
		{
			range.size = desc.numRootConstants * sizeof(u32);
			range.offset = 0;
			range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			cinfo.pushConstantRangeCount = 1;
			cinfo.pPushConstantRanges = &range;
		}

		//VkDescriptorSetLayoutBinding samplerBinding{};
		//samplerBinding.binding = 0;
		//samplerBinding.descriptorCount = 1;
		//samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//samplerBinding.pImmutableSamplers = nullptr;// &gLinearSampler;
		//samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//VkDescriptorSetLayoutCreateInfo layoutInfo{};
		//layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		//layoutInfo.bindingCount = 1;
		//layoutInfo.pBindings = &samplerBinding;

		if (!desc.uniforms.empty())
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = 0;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			samplerLayoutBinding.pImmutableSamplers = nullptr; // Bind sampler here

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &samplerLayoutBinding;

			vkCreateDescriptorSetLayout(gVKDevice, &layoutInfo, gVKGlobalAllocationsCallbacks, &dsLayout);

			cinfo.pSetLayouts = &dsLayout;
			cinfo.setLayoutCount = 1;
		}

		vkCreatePipelineLayout(gVKDevice, &cinfo, gVKGlobalAllocationsCallbacks, &layout);
	}

	cinfo.layout = layout;
	gAllPipelineLayouts.push_back(layout);
	gAllPipelineDSLayouts.push_back(dsLayout);

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

	pipInputAssemblyState.topology = vk_utils::VkPrimitiveTopologyFromErmy(desc.topology);

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

	VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };

	if (gVKConfig.useDynamicRendering)
	{
		pipeline_rendering_create_info.colorAttachmentCount = 1;
		pipeline_rendering_create_info.pColorAttachmentFormats = &gVKSurfaceFormat;
		cinfo.pNext = &pipeline_rendering_create_info;
	}
	else
	{
		if (desc.specificRenderPass.isValid())
		{
			cinfo.renderPass = gAllRenderPasses[desc.specificRenderPass.handle].renderpass;
		}
		else
		{
			cinfo.renderPass = gVKRenderPass; //final pass. TODO:// framegraph passes
		}		
	}


	VkPipeline pipeline = nullptr;
	VK_CALL(vkCreateGraphicsPipelines(gVKDevice, nullptr /*TODO: PipelineCache*/, 1, &cinfo, gVKGlobalAllocationsCallbacks, &pipeline));
	return pipeline;
}

PSOID ermy::rendering::CreatePSO(const PSODesc& desc)
{
	PSOID id{ static_cast<u32>(gAllPipelines.size()) };

	gAllPipelines.push_back(_createPipeline(desc));
	return id;
}

TextureID ermy::rendering::CreateDedicatedTexture(const TextureDesc& desc)
{
	TextureID id{ static_cast<u32>(gAllImages.size()) };

	auto tex = _createTexture(desc);
	gAllImages.push_back(tex.image);
	gAllImageViews.push_back(tex.imageView);
	gAllImageDescriptors.push_back(tex.descriptor);
	gAllImageMetas.push_back(tex.meta);

	return id;
}

BufferID ermy::rendering::CreateDedicatedBuffer(const BufferDesc& desc)
{
	BufferID id{ static_cast<u32>(gAllBuffers.size()) };
	gAllBuffers.push_back(_createBuffer(desc));
	return id;
}

u64 ermy::rendering::GetTextureDescriptor(TextureID tid)
{
	if (!tid.isValid())
		return 0;

	return reinterpret_cast<u64>(gAllImageDescriptors[tid.handle]);
}

RenderPassID ermy::rendering::CreateRenderPass(const RenderPassDesc& desc)
{
	RenderPassID id{ static_cast<u32>(gAllRenderPasses.size()) };
	gAllRenderPasses.push_back(_createRenderPass(desc));
	return id;
}
#endif