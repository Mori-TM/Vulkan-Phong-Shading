VkImage DepthImage;
VkDeviceMemory DepthImageMemory;
VkImageView DepthImageView;
VkDevice DepthImageDevice;
bool DepthImageCreated = false;
//m_Created

//GetImageView = DepthImageView

static VkFormat FindDepthFormat(VkPhysicalDevice PhysicalDevice);

void CreateDepthBuffer(VkDevice Device, VkPhysicalDevice PhysicalDevice, VkCommandPool CommandPool, VkQueue Queue, int width, int height)
{
	if(DepthImageCreated)
	{
		printf("DepthImage was already created\n");
	}
		
	DepthImageDevice = Device;	
	
	VkFormat DepthFormat = FindDepthFormat(PhysicalDevice);
	
	CreateImage(Device, PhysicalDevice, width, height, DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DepthImage, DepthImageMemory);
	CreateImageView(Device, DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, DepthImageView);
	
	ChangeImageLayout(Device, CommandPool, Queue, DepthImage, DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	
	DepthImageCreated = true;	
}

void DestroyDepthBuffer()
{
	if(DepthImageCreated)
	{
		vkDestroyImageView(DepthImageDevice, DepthImageView, NULL);
		vkDestroyImage(DepthImageDevice, DepthImage, NULL);
		vkFreeMemory(DepthImageDevice, DepthImageMemory, NULL);
		DepthImageCreated = false;
	}
}

static VkFormat FindDepthFormat(VkPhysicalDevice PhysicalDevice)
{
	std::vector<VkFormat> PosibleFormats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT};
	
	return FindSupportedFormat(PhysicalDevice, PosibleFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static VkAttachmentDescription GetDepthAttachment(VkPhysicalDevice PhysicalDevice)
{
	VkAttachmentDescription DepthAttachment;
	DepthAttachment.flags = 0;
	DepthAttachment.format = FindDepthFormat(PhysicalDevice);
	DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	return DepthAttachment;
}

static VkPipelineDepthStencilStateCreateInfo GetDepthStencilStateCreateInfoOpaque()
{
	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo;
	DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateCreateInfo.pNext = NULL;
	DepthStencilStateCreateInfo.flags = 0;
	DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	DepthStencilStateCreateInfo.front = {};
	DepthStencilStateCreateInfo.back = {};
	DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
	DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;
	
	return DepthStencilStateCreateInfo;
}
