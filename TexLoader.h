#define STB_IMAGE_IMPLEMENTATION
#include <images/stb_image.h>

unsigned int texture[512];

struct
{
	int width = 0;
	int height = 0;
	int bitdepth = 0;
	int SizeInByte = 0;
}ImageInfo;


stbi_uc *TexData = NULL;

bool Uploaded = false;
VkImage Image;
VkDeviceMemory ImageMemory;
VkImageView ImageView;
VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
VkDevice ImageDevice;
VkSampler Sampler;

//m_image = Image
//m_imageMemory = ImageMemory
//m_ppixels = TexData
//getSampler = Sampler
//getImageView = ImageView

void ChangeLayout(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue, VkImageLayout Layout);
void WriteBufferToImage(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue, VkBuffer Buffer);

void LoadTexture(const char *Path)
{
	stbi_set_flip_vertically_on_load(true);
	
	TexData = stbi_load(Path, &ImageInfo.width, &ImageInfo.height, &ImageInfo.bitdepth, STBI_rgb_alpha);
	ImageInfo.SizeInByte = ImageInfo.width * ImageInfo.height * 4;
	printf("SizeInByte: %d\n", ImageInfo.SizeInByte);
	printf("Width: %d\n", ImageInfo.width);
	printf("Height: %d\n", ImageInfo.height);
	printf("Bitdepth: %d\n", ImageInfo.bitdepth);
	
	if(TexData == NULL) printf("Cannot Open Texture\n");
}

void Upload(VkDevice Device, VkPhysicalDevice PhysicalDevice, VkCommandPool CommandPool, VkQueue Queue)
{	
	ImageDevice = Device;
	
	VkDeviceSize ImageSize = ImageInfo.SizeInByte;
	
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory = 0;
	
	CreateBuffer(Device, PhysicalDevice, ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBufferMemory);
	
	void *data = NULL;
	vkMapMemory(Device, StagingBufferMemory, 0, ImageSize, 0, &data);
	memcpy(data, TexData, ImageSize);
	vkUnmapMemory(Device, StagingBufferMemory);
	
	CreateImage(Device, PhysicalDevice, ImageInfo.width, ImageInfo.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image, ImageMemory);
	
	ChangeLayout(Device, CommandPool, Queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	WriteBufferToImage(Device, CommandPool, Queue, StagingBuffer);
	ChangeLayout(Device, CommandPool, Queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	
	vkDestroyBuffer(Device, StagingBuffer, NULL);
	vkFreeMemory(Device, StagingBufferMemory, VK_NULL_HANDLE);

	CreateImageView(Device, Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, ImageView);
	
	VkSamplerCreateInfo SamplerCreateInfo;
	SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCreateInfo.pNext = NULL;
	SamplerCreateInfo.flags = 0;
	SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCreateInfo.mipLodBias = 0.0f;
	SamplerCreateInfo.anisotropyEnable = VK_TRUE;
	SamplerCreateInfo.maxAnisotropy = 16;
	SamplerCreateInfo.compareEnable = VK_FALSE;
	SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerCreateInfo.minLod = 0.0f;
	SamplerCreateInfo.maxLod = 0.0f;
	SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	
	VkResult result = vkCreateSampler(Device, &SamplerCreateInfo, NULL, &Sampler);
	ASSERT_VULKAN(result);
	
	Uploaded = true;
}

void ChangeLayout(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue, VkImageLayout Layout)
{
	ChangeImageLayout(Device, CommandPool, Queue, Image, VK_FORMAT_R8G8B8A8_UNORM, ImageLayout, Layout);
	
	ImageLayout = Layout;
}

void WriteBufferToImage(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue, VkBuffer Buffer)
{
	VkCommandBuffer CommandBuffer = StartCommandBuffer(Device, CommandPool);	
	
	VkBufferImageCopy BufferImageCopy;
	BufferImageCopy.bufferOffset = 0;
	BufferImageCopy.bufferRowLength = 0;
	BufferImageCopy.bufferImageHeight = 0;
	BufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	BufferImageCopy.imageSubresource.mipLevel = 0;
	BufferImageCopy.imageSubresource.baseArrayLayer = 0;
	BufferImageCopy.imageSubresource.layerCount = 1;
	BufferImageCopy.imageOffset = {0, 0, 0};
	BufferImageCopy.imageExtent = {(uint32_t)ImageInfo.width, (uint32_t)ImageInfo.height, 1};
	
	vkCmdCopyBufferToImage(CommandBuffer, Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
	
	EndCommandBuffer(Device, Queue, CommandPool, CommandBuffer);
}

void DestroyTexture()
{
	stbi_image_free(TexData);
	vkDestroySampler(ImageDevice, Sampler, NULL);
	vkDestroyImageView(ImageDevice, ImageView, NULL);
	
	vkDestroyImage(ImageDevice, Image, NULL);
	vkFreeMemory(ImageDevice, ImageMemory, NULL);
}
