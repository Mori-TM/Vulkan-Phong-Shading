#define ASSERT_VULKAN(val)		\
		if(val != VK_SUCCESS)	\
		{						\
			printf("Error\n");	\
			exit(0);			\
		}	

		
VkInstance instance;
std::vector<VkPhysicalDevice> physicalDevices;
VkSurfaceKHR surface;
VkDevice device;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
VkImageView *imageViews;
VkFramebuffer *Framebuffers;
uint32_t amountOfImagesInSwapchain = 0;
VkShaderModule ShaderModuleVert;
VkShaderModule ShaderModuleFrag;
VkPipelineLayout PipelineLayout;
VkRenderPass RenderPass;
VkPipeline Pipeline;
VkCommandPool CommandPool;
VkCommandBuffer *CommandBuffers;
VkSemaphore SemaphoreImageAvailable;
VkSemaphore SemaphoreRenderingDone;
VkQueue queue;
VkBuffer VertexBuffer;
VkDeviceMemory VertexBufferDeviceMemory;
VkBuffer IndexBuffer;
VkDeviceMemory IndexBufferDeviceMemory;
VkBuffer UniformBuffer;
VkDeviceMemory UniformBufferMemory;

GLFWwindow *window;

uint32_t Width = 800;
uint32_t Height = 800;
const VkFormat BGRA = VK_FORMAT_B8G8R8A8_UNORM;

struct UniformBufferObject
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
	glm::vec3 LightPos;
};
VkDescriptorSetLayout DescriptorSetLayout;
VkDescriptorPool DescriptorPool;
VkDescriptorSet DescriptorSet;

//DepthImage DepthImageBuffer;


uint32_t FindMemoryTypeIndex(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
{
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);
	for(uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if((TypeFilter & (1 << i)) && (PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
		{
			return i;
		}
	}
	
	printf("Found no correct Memory Type");
}

bool IsFormatSupported(VkPhysicalDevice PhysicalDevice, VkFormat Format, VkImageTiling Tiling, VkFormatFeatureFlags FeatureFlags)
{
	VkFormatProperties FormatProperties;
	vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &FormatProperties);
	
	if(Tiling == VK_IMAGE_TILING_LINEAR && (FormatProperties.linearTilingFeatures & FeatureFlags) == FeatureFlags)
	{
		return true;
	}
	else if(Tiling == VK_IMAGE_TILING_OPTIMAL && (FormatProperties.optimalTilingFeatures & FeatureFlags) == FeatureFlags)
	{
		return true;
	}
	
	return false;
}

VkFormat FindSupportedFormat(VkPhysicalDevice PhysicalDevice, const std::vector<VkFormat> &Formats, VkImageTiling Tiling, VkFormatFeatureFlags FeatureFlags)
{
	for(VkFormat Format : Formats)
	{
		if(IsFormatSupported(PhysicalDevice, Format, Tiling, FeatureFlags))
		{
			return Format;
		}
	}	
	
	printf("No Supported Format found\n");
}

bool IsStencilFormat(VkFormat Format)
{
	return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CreateBuffer(VkDevice Device, VkPhysicalDevice PhysicalDevice, VkDeviceSize DeviceSize, VkBufferUsageFlags BufferUsageFlags, VkBuffer &Buffer, VkMemoryPropertyFlags MemoryPropertyFlags, VkDeviceMemory &DeviceMemory)
{
	VkBufferCreateInfo BufferCreateInfo;
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.pNext = NULL;
	BufferCreateInfo.flags = 0;
	BufferCreateInfo.size = DeviceSize;
	BufferCreateInfo.usage = BufferUsageFlags;
	BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	BufferCreateInfo.queueFamilyIndexCount = 0;
	BufferCreateInfo.pQueueFamilyIndices = NULL;
	
	VkResult result = vkCreateBuffer(device, &BufferCreateInfo, NULL, &Buffer);
	ASSERT_VULKAN(result);
	
	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(device, Buffer, &MemoryRequirements);
	
	VkMemoryAllocateInfo MemoryAllocateInfo;
	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.pNext = NULL;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	MemoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(PhysicalDevice, MemoryRequirements.memoryTypeBits, MemoryPropertyFlags);

	result = vkAllocateMemory(device, &MemoryAllocateInfo, NULL, &DeviceMemory);
	ASSERT_VULKAN(result);
	
	vkBindBufferMemory(device, Buffer, DeviceMemory, 0);
}

VkCommandBuffer StartCommandBuffer(VkDevice Device, VkCommandPool CommandPool)
{
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo;
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.pNext = NULL;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	
	VkCommandBuffer CommandBuffer;
	VkResult result = vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CommandBuffer);
	ASSERT_VULKAN(result);
	
	VkCommandBufferBeginInfo CommandBufferBeginInfo;
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.pNext = NULL;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CommandBufferBeginInfo.pInheritanceInfo = NULL;
	
	result = vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);
	ASSERT_VULKAN(result);
	
	return CommandBuffer;
}

void EndCommandBuffer(VkDevice Device, VkQueue Queue, VkCommandPool CommandPool, VkCommandBuffer CommandBuffer)
{
	VkResult result = vkEndCommandBuffer(CommandBuffer);
	ASSERT_VULKAN(result);
	
	VkSubmitInfo SubmitInfo;
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pNext = NULL;
	SubmitInfo.waitSemaphoreCount = 0;
	SubmitInfo.pWaitSemaphores = NULL;
	SubmitInfo.pWaitDstStageMask = NULL;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;
	SubmitInfo.signalSemaphoreCount = 0;
	SubmitInfo.pSignalSemaphores = NULL;
	
	result = vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE);
	ASSERT_VULKAN(result);
	
	vkQueueWaitIdle(Queue);
	
	vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
}

void CreateImage(VkDevice Device, VkPhysicalDevice PhysicalDevice, int width, int height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags, VkImage &Image, VkDeviceMemory &ImageMemory)
{
	VkImageCreateInfo ImageCreateInfo;
	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.pNext = NULL;
	ImageCreateInfo.flags = 0;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.format = Format;
	ImageCreateInfo.extent.width = width;
	ImageCreateInfo.extent.height = height;
	ImageCreateInfo.extent.depth = 1;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.tiling = Tiling;
	ImageCreateInfo.usage = UsageFlags;	
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.queueFamilyIndexCount = 0;
	ImageCreateInfo.pQueueFamilyIndices = NULL;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	
	VkResult result = vkCreateImage(Device, &ImageCreateInfo, NULL, &Image);
	ASSERT_VULKAN(result);
	
	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements(Device, Image, &MemoryRequirements);
	
	VkMemoryAllocateInfo MemoryAllocateInfo;
	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.pNext = NULL;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	MemoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(PhysicalDevice, MemoryRequirements.memoryTypeBits, PropertyFlags);
	
	result = vkAllocateMemory(Device, &MemoryAllocateInfo, NULL, &ImageMemory);
	ASSERT_VULKAN(result);
	
	vkBindImageMemory(Device, Image, ImageMemory, 0);
}

void CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, VkImageView &ImageView)
{
	VkImageViewCreateInfo ImageViewCreateInfo;
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.pNext = NULL;
	ImageViewCreateInfo.flags = 0;
	ImageViewCreateInfo.image = Image;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.format = Format;
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		//If Not working VK_COMPONENT_SWIZZLE_IDENTITY
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.aspectMask = AspectFlags;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	
	VkResult result = vkCreateImageView(Device, &ImageViewCreateInfo, NULL, &ImageView);
	ASSERT_VULKAN(result);
}

void CopyBuffer(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue, VkBuffer Src, VkBuffer Dest, VkDeviceSize Size)
{
	VkCommandBuffer CommandBuffer = StartCommandBuffer(Device, CommandPool);
	
	VkBufferCopy BufferCopy;
	BufferCopy.srcOffset = 0;
	BufferCopy.dstOffset = 0;
	BufferCopy.size = Size;
	vkCmdCopyBuffer(CommandBuffer, Src, Dest, 1, &BufferCopy);
	
	EndCommandBuffer(Device, Queue, CommandPool, CommandBuffer);
}

template <typename T>
void CreateAndUploadBuffer(VkDevice Device, VkPhysicalDevice PhysicalDevice, VkQueue Queue, VkCommandPool CommandPool, std::vector<T> Data, VkBufferUsageFlags Usage, VkBuffer &Buffer, VkDeviceMemory &DeviceMemory)
{
	VkDeviceSize BufferSize = sizeof(T) * Data.size();
	
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;
	CreateBuffer(Device, PhysicalDevice, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBufferMemory);
	
	
	void *RawData;
	vkMapMemory(device, StagingBufferMemory, 0, BufferSize, 0, &RawData);
	memcpy(RawData, Data.data(), BufferSize);
	vkUnmapMemory(device, StagingBufferMemory);
	
	CreateBuffer(Device, PhysicalDevice, BufferSize, Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DeviceMemory);
	
	CopyBuffer(Device, CommandPool, Queue, StagingBuffer, Buffer, BufferSize);
	
	vkDestroyBuffer(device, StagingBuffer, NULL);
	vkFreeMemory(device, StagingBufferMemory, NULL);
}

void ChangeImageLayout(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout)
{
	VkCommandBuffer CommandBuffer = StartCommandBuffer(Device, CommandPool);	
	
	VkImageMemoryBarrier ImageMemoryBarrier;
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImageMemoryBarrier.pNext = NULL;
	if(OldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if(OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if(OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		ImageMemoryBarrier.srcAccessMask = 0;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else
	{
		printf("Layout transition not supported");
	}
	
	ImageMemoryBarrier.oldLayout = OldLayout;
	ImageMemoryBarrier.newLayout = NewLayout;
	ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.image = Image;
	if(NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		
		if(IsStencilFormat(Format))
		{
			ImageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	ImageMemoryBarrier.subresourceRange.levelCount = 1;
	ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	ImageMemoryBarrier.subresourceRange.layerCount = 1;
	
	vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);
	
	EndCommandBuffer(Device, Queue, CommandPool, CommandBuffer);
}
