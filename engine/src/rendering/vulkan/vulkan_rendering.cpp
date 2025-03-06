#include "vulkan_rendering.h"
#include "vulkan_swapchain.h"
#ifdef ERMY_GAPI_VULKAN
#include "vk_utils.h"
#include "ermy_rendering.h"

using namespace ermy;
using namespace ermy::rendering;

struct ShaderRelatedPSO
{
	ermy::ShaderStage stage = ermy::ShaderStage::Vertex;
	u64 shaderHash = 0;
	std::vector<int> psoIndices;
};

std::unordered_map<std::string, ShaderRelatedPSO> gShaderRelatedPSOs;
std::vector<ermy::rendering::PSODesc> gAllPipelineDescs;

std::vector<VkPipeline> gAllPipelines;
std::vector<VkPipelineLayout> gAllPipelineLayouts;
std::vector<VkDescriptorSetLayout> gAllPipelineDSLayouts;
std::vector<VkShaderModule> gAllShaderModules;

std::vector<VkBuffer> gAllBuffers;
std::vector<VkDescriptorSet> gAllBufferDescriptors;
std::vector<BufferMeta> gAllBufferMetas;

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

struct BufferInfo {
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo allocationInfo = {};
	VkDescriptorSet descriptor = VK_NULL_HANDLE;
	BufferMeta meta;
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

void _addShader(const ermy::ShaderInfo& sinfo, std::vector<VkPipelineShaderStageCreateInfo>& stages)
{
	if (sinfo.byteCode.size == 0)
		return;

	if (!sinfo.byteCode.cachedDeviceObjectID.isValid())
	{
		sinfo.byteCode.cachedDeviceObjectID.handle = static_cast<u16>(gAllShaderModules.size());

		VkShaderModuleCreateInfo cinfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		cinfo.pCode = (const uint32_t*)sinfo.byteCode.data;
		cinfo.codeSize = sinfo.byteCode.size;

		VkShaderModule& shaderModule = gAllShaderModules.emplace_back();
		VK_CALL(vkCreateShaderModule(gVKDevice, &cinfo, gVKGlobalAllocationsCallbacks, &shaderModule));

		vk_utils::debug::SetName(shaderModule, sinfo.shaderName.c_str());
		//auto entryPoint = _extractEntryPointName((const uint32_t*)bc.data, bc.size / 4);
		//gAllShaderModulesEntryPoints.push_back(entryPoint);
	}

	auto& newStage = stages.emplace_back();
	newStage.module = gAllShaderModules[sinfo.byteCode.cachedDeviceObjectID.handle];
	newStage.stage = vk_utils::VkShaderStageFromErmy(sinfo.byteCode.stage);
	newStage.pName = "main";
	newStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
}

RenderpassInfo _createRenderPass(const RenderPassDesc& desc)
{
	RenderpassInfo result = {};
	const ImageMeta& meta = gAllImageMetas[desc.colorAttachment.handle];
	VkImage image = gAllImages[desc.colorAttachment.handle];
	VkImageView imageView = gAllImageViews[desc.colorAttachment.handle];

	ImageMeta metaDepth = {};
	VkImage imageDepth = {};
	VkImageView imageViewDepth = {};

	if (desc.depthStencilAttachment.isValid())
	{
		metaDepth = gAllImageMetas[desc.depthStencilAttachment.handle];
		imageDepth = gAllImages[desc.depthStencilAttachment.handle];
		imageViewDepth = gAllImageViews[desc.depthStencilAttachment.handle];
	}

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

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = metaDepth.format;
	depthAttachment.samples = metaDepth.samples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkImageView> attachmentsViews;

	int colorAttachmentIndex = 0;

	if (desc.depthStencilAttachment.isValid())
	{
		colorAttachmentIndex = 1;
		attachments.push_back(depthAttachment);
		attachmentsViews.push_back(imageViewDepth);
		result.useDepth = true;
	}

	colorAttachmentRef.attachment = colorAttachmentIndex;
	attachments.push_back(colorAttachment);
	attachmentsViews.push_back(imageView);

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = result.useDepth ? &depthAttachmentRef : nullptr;
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

	rpass.attachmentCount = attachments.size();
	rpass.pAttachments = attachments.data();
	rpass.subpassCount = 1;
	rpass.pSubpasses = &subpass;
	rpass.dependencyCount = 0;
	rpass.pDependencies = &dependency;

	vkCreateRenderPass(gVKDevice, &rpass, gVKGlobalAllocationsCallbacks, &result.renderpass);

	VkFramebufferCreateInfo fbuff = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };

	fbuff.renderPass = result.renderpass; // Associate the framebuffer with the render pass
	fbuff.attachmentCount = attachmentsViews.size();
	fbuff.pAttachments = attachmentsViews.data(); // Use the provided VkImageView
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
	//VkDeviceSize imageSize = desc.width * desc.height * 4; // 4 channels (RGBA) TODO: from format
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferAllocation;
	void* pixelsCPUdata = nullptr;
	ImageMeta meta;
	auto formatInfo = ermy::rendering::GetFormatInfo(desc.texelFormat);

	VkImageUsageFlags textureUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	bool isDepthFormat = ermy::rendering::IsDepthFormat(desc.texelFormat);

	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (isDepthFormat)
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

	if (desc.pixelsData)
	{
		textureUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else
	{
		if (isDepthFormat)
			textureUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			textureUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	meta.width = static_cast<uint32_t>(desc.width);
	meta.height = static_cast<uint32_t>(desc.height);
	meta.format = vk_utils::VkFormatFromErmy(desc.texelFormat);
	meta.samples = VK_SAMPLE_COUNT_1_BIT;
	meta.depth = static_cast<uint32_t>(desc.depth);

	VkImageType imgType = VK_IMAGE_TYPE_2D;
	if (desc.height == 1 && desc.width > 1 && desc.depth == 1)
		imgType = VK_IMAGE_TYPE_1D;
	if (desc.depth > 1)
		imgType = VK_IMAGE_TYPE_3D;


	// Create a Vulkan image using VMA
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	if (desc.isCubemap)
		imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	//if (desc.numLayers > 1 && !(desc.numLayers && desc.isCubemap))
	//	imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	imageInfo.imageType = imgType;
	imageInfo.extent.width = static_cast<uint32_t>(desc.width);
	imageInfo.extent.height = static_cast<uint32_t>(desc.height);
	imageInfo.extent.depth = static_cast<uint32_t>(desc.depth);
	imageInfo.mipLevels = static_cast<uint32_t>(desc.numMips);
	imageInfo.arrayLayers = static_cast<uint32_t>(desc.numLayers);
	imageInfo.format = meta.format; 
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = textureUsage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo imageAllocCreateInfo{};
	imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImage textureImage = VK_NULL_HANDLE;
	VmaAllocation textureImageAllocation = VK_NULL_HANDLE;
	VmaAllocationInfo textureImageAllocationInfo;
	VK_CALL(vmaCreateImage(gVMA_Allocator, &imageInfo, &imageAllocCreateInfo, &textureImage, &textureImageAllocation, &textureImageAllocationInfo));

	auto c = AllocateSingleTimeCommandBuffer();

	if (desc.pixelsData)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = desc.dataSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		VmaAllocationInfo bufferAllocationInfo;
		VK_CALL(vmaCreateBuffer(gVMA_Allocator, &bufferInfo, &allocCreateInfo, &stagingBuffer, &stagingBufferAllocation, &bufferAllocationInfo));

		vmaMapMemory(gVMA_Allocator, stagingBufferAllocation, &pixelsCPUdata);
		memcpy(pixelsCPUdata, desc.pixelsData, static_cast<size_t>(desc.dataSize));
		vmaUnmapMemory(gVMA_Allocator, stagingBufferAllocation);

		vk_utils::ImageTransition(c.cbuff, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aspect, desc.numMips,desc.numLayers);

		u32 bufferOffset = 0;
		std::vector<VkBufferImageCopy> allRegions;
		allRegions.reserve(desc.numLayers * desc.numMips);
		int  blockPixels = formatInfo.blockWidth * formatInfo.blockHeight * formatInfo.blockDepth;

		for (int l = 0; l < desc.numLayers; ++l)
		{
			u32 mipWidth = desc.width;
			u32 mipHeight = desc.height;
			u32 mipDepth = desc.depth;

			for (int m = 0; m < desc.numMips; ++m)
			{
				VkBufferImageCopy& region = allRegions.emplace_back();
				region.bufferOffset = bufferOffset;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = m;
				region.imageSubresource.baseArrayLayer = l;
				region.imageSubresource.layerCount = 1;
				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { mipWidth, mipHeight, mipDepth };

				int currentMipDataSize = mipWidth * mipHeight * mipDepth * formatInfo.blockSize / blockPixels;
				bufferOffset += currentMipDataSize;

				if(mipWidth > formatInfo.blockWidth)
					mipWidth /= 2;

				if (mipHeight > formatInfo.blockHeight)
					mipHeight /= 2;

				if (mipDepth > formatInfo.blockDepth)
					mipDepth /= 2;
			}
		}

		vkCmdCopyBufferToImage(c.cbuff, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(allRegions.size()), allRegions.data());


		vk_utils::ImageTransition(c.cbuff, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aspect, desc.numMips, desc.numLayers);
	}
	else
	{
		vk_utils::ImageTransition(c.cbuff, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aspect, desc.numMips, desc.numLayers);
	}

	c.Sumbit();

	VkImageViewType imgViewType = VK_IMAGE_VIEW_TYPE_2D;

	if (desc.depth > 1)
	{
		imgViewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else if (desc.height == 1 && desc.width > 1 && desc.depth == 1)
	{
		imgViewType = VK_IMAGE_VIEW_TYPE_1D;
		if (desc.numLayers > 1)
			imgViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	}
	else
	{
		if (desc.isCubemap)
		{
			if (desc.numLayers > 6)
				imgViewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			else
				imgViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else
		{
			if (desc.numLayers > 1)
				imgViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
	}

	// Create an image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = imgViewType;
	viewInfo.format = meta.format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = desc.numMips;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = desc.numLayers;

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

	if (desc.debugName)
	{
		vk_utils::debug::SetName(textureImage, desc.debugName);

		std::string debugViewName = desc.debugName;
		debugViewName += "_View";
		vk_utils::debug::SetName(textureImageView, debugViewName.c_str());
	}
		

	return result;
}

BufferInfo _createBuffer(const BufferDesc& desc) {
	// Create the buffer in GPU memory
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = desc.size; // Size of the buffer
	bufferInfo.usage = vk_utils::VkBufferUsageFromErmy(desc.usage) | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // Usage flags (e.g., VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO; // Allocate in GPU memory
	
	if (desc.usage == BufferUsage::Uniform)
	{
		allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		allocCreateInfo.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		allocCreateInfo.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo allocationInfo;
	BufferMeta meta = { };

	VK_CALL(vmaCreateBuffer(gVMA_Allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));
	meta.size = desc.size;
	meta.bufferUsage = desc.usage;

	// If initial data is provided, use a staging buffer to copy data to the GPU buffer
	if (desc.initialData || desc.persistentMapped) {
		// Create a staging buffer in CPU-visible memory
		VkBufferCreateInfo stagingBufferInfo{};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.size = desc.size;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo stagingAllocCreateInfo{};
		stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VK_CALL(vmaCreateBuffer(gVMA_Allocator, &stagingBufferInfo, &stagingAllocCreateInfo, &meta.stagingBuffer, &meta.stagingBufferAllocation, nullptr));

		// Map the staging buffer and copy the data
		vmaMapMemory(gVMA_Allocator, meta.stagingBufferAllocation, &meta.presistentMappedPtr);

		if (desc.initialData != nullptr)
		{
			memcpy(meta.presistentMappedPtr, desc.initialData, static_cast<size_t>(desc.size));
		}
		
		if (!desc.persistentMapped)
		{
			vmaUnmapMemory(gVMA_Allocator, meta.stagingBufferAllocation);
		}
		
		// Use a command buffer to copy data from the staging buffer to the GPU buffer
		auto c = AllocateSingleTimeCommandBuffer();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = desc.size;
		vkCmdCopyBuffer(c.cbuff, meta.stagingBuffer, buffer, 1, &copyRegion);

		c.Sumbit();

		// Clean up the staging buffer
		//vmaDestroyBuffer(gVMA_Allocator, stagingBuffer, stagingBufferAllocation);
	}
	VkDescriptorSet descriptor = VK_NULL_HANDLE;

	if (desc.usage == BufferUsage::Uniform)
	{
		// Create a descriptor set for the texture
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = gStaticDescriptorsPool; // Assume this is created elsewhere
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &gCanvasDomainLayout; // Assume this is created elsewhere

		VK_CALL(vkAllocateDescriptorSets(gVKDevice, &allocInfo, &descriptor));

		VkDescriptorBufferInfo bufferInfoDesc{};
		bufferInfoDesc.buffer = buffer;
		bufferInfoDesc.offset = 0;
		bufferInfoDesc.range = desc.size;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptor;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfoDesc;

		vkUpdateDescriptorSets(gVKDevice, 1, &descriptorWrite, 0, nullptr);

		vmaMapMemory(gVMA_Allocator, allocation, &meta.presistentMappedPtr);
	}
	

	if (desc.debugName)
	{
		vk_utils::debug::SetName(buffer, desc.debugName);
		
		if (meta.stagingBuffer)
		{
			vk_utils::debug::SetName(meta.stagingBuffer, "%s_STAGING", desc.debugName);
		}		
	}

	// Return the buffer and its allocation information
	BufferInfo result;
	result.buffer = buffer;
	result.allocation = allocation;
	result.allocationInfo = allocationInfo;
	result.descriptor = descriptor;
	result.meta = meta;
	return result;
}

VkDescriptorSetLayout _createCanvasDSLayout()
{
	VkDescriptorSetLayoutBinding frsameConstantsBinding = {};
	frsameConstantsBinding.binding = 0;
	frsameConstantsBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	frsameConstantsBinding.descriptorCount = 1;
	frsameConstantsBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	frsameConstantsBinding.pImmutableSamplers = nullptr; // Bind sampler here

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &frsameConstantsBinding;
	
	VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
	vkCreateDescriptorSetLayout(gVKDevice, &layoutInfo, gVKGlobalAllocationsCallbacks, &dsLayout);

	return dsLayout;
}

VkDescriptorSetLayout _createSceneDSLayout()
{
	VkDescriptorSetLayoutBinding frsameConstantsBinding[2] = {};
	frsameConstantsBinding[0].binding = 0;
	frsameConstantsBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	frsameConstantsBinding[0].descriptorCount = 1;
	frsameConstantsBinding[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	frsameConstantsBinding[0].pImmutableSamplers = nullptr; // Bind sampler here

	frsameConstantsBinding[1].binding = 1;
	frsameConstantsBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	frsameConstantsBinding[1].descriptorCount = 1;
	frsameConstantsBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	frsameConstantsBinding[1].pImmutableSamplers = nullptr; // Bind sampler here

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = frsameConstantsBinding;

	VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
	vkCreateDescriptorSetLayout(gVKDevice, &layoutInfo, gVKGlobalAllocationsCallbacks, &dsLayout);

	return dsLayout;
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
		std::vector<VkPushConstantRange> pushConstantRanges;
		int maxRange = 0;

		for (int i = 0; i < (int)ShaderStage::MAX; ++i)
		{
			if (desc.rootConstantRanges[i].size > 0)
			{
				maxRange = std::max(maxRange, desc.rootConstantRanges[i].offset + desc.rootConstantRanges[i].size);	

				//VkPushConstantRange& range = pushConstantRanges.emplace_back();

				//range.size = desc.rootConstantRanges[i].size;
				//range.offset = desc.rootConstantRanges[i].offset;
				//range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;// vk_utils::VkShaderStageFromErmy((ShaderStage)i);
			}
		}


		if (maxRange > 0)
		{
			VkPushConstantRange& range = pushConstantRanges.emplace_back();
			range.size = maxRange;
			range.offset = 0;
			range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		}


		cinfo.pushConstantRangeCount = static_cast<u32>(pushConstantRanges.size());
		cinfo.pPushConstantRanges = pushConstantRanges.data();

		std::vector<VkDescriptorSetLayout> dsLayouts;

		if (desc.domain == PSODomain::Canvas)
		{
			static VkDescriptorSetLayout frameDSLayout = _createCanvasDSLayout();
			dsLayouts.push_back(frameDSLayout);
		}

		if (desc.domain == PSODomain::Scene)
		{
			static VkDescriptorSetLayout frameDSLayout = _createSceneDSLayout();
			dsLayouts.push_back(frameDSLayout);
		}

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
			dsLayouts.push_back(dsLayout);
		}

		cinfo.pSetLayouts = dsLayouts.data();
		cinfo.setLayoutCount = (u32)dsLayouts.size();

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

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(StaticVertexDedicated);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

	int currentOffset = 0;
	int location = 0;
	if(!desc.vertexAttributes.empty())
	{
		for (auto& d : desc.vertexAttributes)
		{
			VkVertexInputAttributeDescription& ad = attributeDescriptions.emplace_back();
			ad.binding = 0;
			ad.location = location++;
			ad.format = vk_utils::VkFormatFromErmy(d.format);
			ad.offset = currentOffset;
			
			auto const &fi = ermy::rendering::GetFormatInfo(d.format);

			currentOffset += fi.blockSize;
		}
		
		pipVertexInputState.vertexBindingDescriptionCount = 1;
		pipVertexInputState.pVertexBindingDescriptions = &bindingDescription;
		pipVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		pipVertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();
	}

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
	for (auto& d : desc.allShaderStages)
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

	std::vector<VkDynamicState> all_dinamic_states;

	all_dinamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	all_dinamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);


	pipDepthStencilState.depthWriteEnable = desc.writeDepth;
	pipDepthStencilState.depthTestEnable = desc.testDepth;
	pipDepthStencilState.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;

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
			const auto& rpass = gAllRenderPasses[desc.specificRenderPass.handle];
			cinfo.renderPass = rpass.renderpass;
			pipMultisamplingState.rasterizationSamples = rpass.samples;
		}
		else
		{
			pipMultisamplingState.rasterizationSamples = gVKSurfaceSamples;
			cinfo.renderPass = gVKRenderPass; //final pass. TODO:// framegraph passes
		}
	}


	VkPipeline pipeline = nullptr;
	VK_CALL(vkCreateGraphicsPipelines(gVKDevice, nullptr /*TODO: PipelineCache*/, 1, &cinfo, gVKGlobalAllocationsCallbacks, &pipeline));

	if (desc.debugName)
	{
		vk_utils::debug::SetName(pipeline, desc.debugName);

		std::string debugLayoutName = desc.debugName;
		debugLayoutName += "_Layout";
		vk_utils::debug::SetName(layout, debugLayoutName.c_str());
	}
	return pipeline;
}

PSOID ermy::rendering::CreatePSO(const PSODesc& desc)
{
	PSOID id{ static_cast<u32>(gAllPipelines.size()) };

	auto PSO = _createPipeline(desc);
	gAllPipelines.push_back(PSO);
	gAllPipelineDescs.push_back(desc);

	for (auto& s : desc.allShaderStages)
	{
		if(s.byteCode.size == 0)
			continue;

		auto it = gShaderRelatedPSOs.find(s.shaderName);
		if (it == gShaderRelatedPSOs.end())
		{
			ShaderRelatedPSO newPSO;
			newPSO.shaderHash = s.bytecodeCRC64;
			newPSO.psoIndices.push_back(id.handle);
			newPSO.stage = s.byteCode.stage;
			gShaderRelatedPSOs[s.shaderName] = newPSO;
		}
		else
		{
			if (it->second.shaderHash != s.bytecodeCRC64)
			{
				it->second.psoIndices.clear();
			}

			it->second.psoIndices.push_back(id.handle);
		}
	}
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
	auto buf = _createBuffer(desc);

	gAllBuffers.push_back(buf.buffer);
	gAllBufferMetas.push_back(buf.meta);
	gAllBufferDescriptors.push_back(buf.descriptor);

	return id;
}

void ermy::rendering::UpdateBufferData(BufferID bid, const void* data)
{
	if (!bid.isValid())
		return;

	auto& buffer = gAllBufferMetas[bid.handle];

	memcpy(buffer.presistentMappedPtr, data, buffer.size);
}

u64 ermy::rendering::GetTextureDescriptor(TextureID tid)
{
	if (!tid.isValid())
		return 0;

	return reinterpret_cast<u64>(gAllImageDescriptors[tid.handle]);
}

u64 ermy::rendering::GetBufferDescriptor(BufferID bid)
{
	if (!bid.isValid())
		return 0;

	return reinterpret_cast<u64>(gAllBufferDescriptors[bid.handle]);
}

RenderPassID ermy::rendering::CreateRenderPass(const RenderPassDesc& desc)
{
	RenderPassID id{ static_cast<u16>(gAllRenderPasses.size()) };
	gAllRenderPasses.push_back(_createRenderPass(desc));
	return id;
}

void ermy::rendering::UpdateShaderBytecode(ShaderDomainTag tag, const std::string& name, u64 bytecodeCRC, const u8* bytecode, u32 bytecodeSize)
{
	auto it = gShaderRelatedPSOs.find(name);

	if (it != gShaderRelatedPSOs.end())
	{
		if (it->second.shaderHash != bytecodeCRC) //updated shader
		{
			for (auto& psoIndex : it->second.psoIndices)
			{
				auto& pso = gAllPipelines[psoIndex];
				//vkDestroyPipeline(gVKDevice, pso, gVKGlobalAllocationsCallbacks);
				//TODO: destroy after safe 3 frames

				auto& shader = gAllPipelineDescs[psoIndex].allShaderStages[(int)it->second.stage];
				auto& shaderBytecode = shader.byteCode;

				shaderBytecode.data = bytecode;
				shaderBytecode.size = bytecodeSize;
				shaderBytecode.cachedDeviceObjectID.Invalidate();
				shader.bytecodeCRC64 = bytecodeCRC;

				pso = _createPipeline(gAllPipelineDescs[psoIndex]);
			}
		}
		else
		{
			it->second.psoIndices.clear();
		}
	}
}

void rendering::UpdateBufferData(BufferID buffer, u32 offset, void* data, u16 dataSize)
{
	auto c = AllocateSingleTimeCommandBuffer();
	vkCmdUpdateBuffer(c.cbuff, gAllBuffers[buffer.handle],offset,dataSize,data);

	auto& meta = gAllBufferMetas[buffer.handle];
	VkAccessFlags accessFlags = VK_ACCESS_MEMORY_READ_BIT;

	if (meta.bufferUsage == rendering::BufferUsage::Index)
		accessFlags = VK_ACCESS_INDEX_READ_BIT;
	if (meta.bufferUsage == rendering::BufferUsage::Vertex)
		accessFlags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (meta.bufferUsage == rendering::BufferUsage::Uniform)
		accessFlags = VK_ACCESS_UNIFORM_READ_BIT;
	//
	// Barrier to ensure write completes before read
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = accessFlags;

	vkCmdPipelineBarrier(c.cbuff,
		VK_PIPELINE_STAGE_TRANSFER_BIT,    // Write stage
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, // Read stage
		0, 1, &barrier, 0, nullptr, 0, nullptr);

	c.Sumbit();
}

ermy::u64 ermy::rendering::CreateDescriptorSet(PSODomain domain, const DescriptorSetDesc& desc)
{
	VkDescriptorSet descriptor = VK_NULL_HANDLE;
	VkDescriptorSetLayout toLayout = VK_NULL_HANDLE;

	if (domain == PSODomain::Canvas)
		toLayout = gCanvasDomainLayout;

	if (domain == PSODomain::Scene)
		toLayout = gSceneDomainLayout;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = gStaticDescriptorsPool; // Assume this is created elsewhere
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &toLayout; // Assume this is created elsewhere

	VK_CALL(vkAllocateDescriptorSets(gVKDevice, &allocInfo, &descriptor));

	std::vector<VkDescriptorBufferInfo> allBufferInfos;
	std::vector<VkDescriptorImageInfo> allImageInfos;
	std::vector< VkWriteDescriptorSet> allWrites;
	allBufferInfos.reserve(desc.allBindings.size());
	allImageInfos.reserve(desc.allBindings.size());
	allWrites.reserve(desc.allBindings.size());

	for (const auto& b : desc.allBindings)
	{
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptor;
		descriptorWrite.dstBinding = b.bindingSlot;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorCount = 1;

		if (b.uniformType == ShaderUniformType::UniformBuffer)
		{
			int handle = b.uniformBuffer.handle;
			VkDescriptorBufferInfo bufferInfoDesc{};
			bufferInfoDesc.buffer = gAllBuffers[handle];
			bufferInfoDesc.offset = 0;
			bufferInfoDesc.range = gAllBufferMetas[handle].size;
			allBufferInfos.push_back(bufferInfoDesc);

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.pBufferInfo = &allBufferInfos[allBufferInfos.size()-1];
		}

		if (b.uniformType == ShaderUniformType::Texture2D
			|| b.uniformType == ShaderUniformType::TextureCube)
		{
			int handle = b.texture.handle;
			VkDescriptorImageInfo imageInfoDesc{};
			imageInfoDesc.imageView = gAllImageViews[handle];
			imageInfoDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfoDesc.sampler = gLinearSampler;
			allImageInfos.push_back(imageInfoDesc);

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.pImageInfo = &allImageInfos[allImageInfos.size() - 1];
		}

		allWrites.push_back(descriptorWrite);
	}
	
	vkUpdateDescriptorSets(gVKDevice, (u32)allWrites.size(), allWrites.data(), 0, nullptr);

	return ermy::u64(descriptor);
}

void ermy::rendering::UpdateDescriptorSet(ermy::u64 ds, const DescriptorSetDesc::Binding& b)
{
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = (VkDescriptorSet)ds;
	descriptorWrite.dstBinding = b.bindingSlot;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;

	VkDescriptorImageInfo imageInfoDesc{};

	if (b.uniformType == ShaderUniformType::Texture2D
		|| b.uniformType == ShaderUniformType::TextureCube)
	{
		int handle = b.texture.handle;	
		imageInfoDesc.imageView = gAllImageViews[handle];
		imageInfoDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfoDesc.sampler = gLinearSampler;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pImageInfo = &imageInfoDesc;
	}

	vkUpdateDescriptorSets(gVKDevice, 1, &descriptorWrite, 0, nullptr);
}

void ermy::rendering::WaitDeviceIdle()
{
	vkDeviceWaitIdle(gVKDevice);
}

#endif