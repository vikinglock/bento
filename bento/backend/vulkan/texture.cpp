#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"
#include <iostream>
#include "../utils.h"

#include <limits.h>
#include <string>

#ifdef WINDOWS
#include <windows.h>
#endif

Texture::Texture(){}

Texture::Texture(std::string fpath,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,const glm::vec4& borderColor,TextureType texT){    
    filepath = getExecutablePath()+"/"+std::string(fpath);
    unsigned char* data = stbi_load((filepath).c_str(),&size.x,&size.y,&channels,STBI_rgb_alpha);
    if(!data)std::cerr << "couldn't load " <<filepath<<"!!!!!"<<std::endl;
    texType = texT;


    setTexture(size,texType);
    setData(data);
    
    changeSampler(wrap,minFilter,magFilter,borderColor);

}
void Texture::setTexture(glm::ivec2 size,TextureType texType,bool mipmapped){
    //if(texture)vkDestroyImage(device,texture,nullptr);
    VkDeviceSize bufferSize = size.x*size.y*4;//so we got mr. mtlbuffer over here
    Bento::createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer,stagingBufferMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = size.x;
    imageInfo.extent.height = size.y;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if(vkCreateImage(device,&imageInfo,nullptr,&texture)!=VK_SUCCESS)
        std::cout<<"couldn't create texture"<<std::endl;
}
void Texture::setData(void* data){
    //if(textureMemory)vkFreeMemory(device,textureMemory,nullptr);
    
    VkDeviceSize bufferSize = size.x*size.y*4;
    void* dat;
    vkMapMemory(device,stagingBufferMemory,0,bufferSize,0,&dat);
    memcpy(dat,data,bufferSize);
    vkUnmapMemory(device,stagingBufferMemory);
    stbi_image_free(data);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device,texture,&memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for(uint32_t i=0;i<memProperties.memoryTypeCount;i++)
        if(memRequirements.memoryTypeBits&(1<<i)&&
        (memProperties.memoryTypes[i].propertyFlags&VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)==VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            allocInfo.memoryTypeIndex = i;

    if(vkAllocateMemory(device,&allocInfo,nullptr,&textureMemory)!=VK_SUCCESS)
        std::cout<<"couldn't allocate texture memory"<<std::endl;

    vkBindImageMemory(device,texture,textureMemory,0);

    transitionImageLayout(texture,VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer,texture,static_cast<uint32_t>(size.x),static_cast<uint32_t>(size.y));
    transitionImageLayout(texture,VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device,stagingBuffer,nullptr);
    vkFreeMemory(device,stagingBufferMemory,nullptr);

    //if(imageView)vkDestroyImageView(device,imageView,nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if(vkCreateImageView(device,&viewInfo,nullptr,&imageView)!=VK_SUCCESS)
        std::cout<<"couldn't create texture image view"<<std::endl;
}
void Texture::changeSampler(WrapMode wrap,MinFilter minFilter,MagFilter magFilter,const glm::vec4& borderColor){
    //if(sampler)vkDestroySampler(device,sampler,nullptr);
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;//VK_FILTER_NEAREST

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    //VK_SAMPLER_ADDRESS_MODE_REPEAT
    //VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
    //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    //VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
    //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER

    // VkPhysicalDeviceProperties properties{};
    // vkGetPhysicalDeviceProperties(physicalDevice,&properties);

    //samplerInfo.anisotropyEnable = VK_TRUE;
    //samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if(vkCreateSampler(device,&samplerInfo,nullptr,&sampler)!=VK_SUCCESS)
        std::cout<<"couldn't create texture sampler"<<std::endl;
}
Texture::~Texture(){
    vkDestroyImage(device,texture,nullptr);
    vkFreeMemory(device,textureMemory,nullptr);
    vkDestroyImageView(device,imageView,nullptr);
    vkDestroySampler(device,sampler,nullptr);
}

void Texture::transitionImageLayout(VkImage image,VkFormat format,VkImageLayout oldLayout,VkImageLayout newLayout){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device,&allocInfo,&commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer,&beginInfo);


    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(oldLayout==VK_IMAGE_LAYOUT_UNDEFINED&&newLayout==VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }else if(oldLayout==VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL&&newLayout==VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }else std::cout<<"layout transition unsupported"<<std::endl;

    vkCmdPipelineBarrier(commandBuffer,sourceStage,destinationStage,0,0,nullptr,0,nullptr,1,&barrier);


    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue,1,&submitInfo,VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device,commandPool,1,&commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer,VkImage image,uint32_t width,uint32_t height){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device,&allocInfo,&commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer,&beginInfo);


    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0,0,0};
    region.imageExtent = {static_cast<uint32_t>(size.x),static_cast<uint32_t>(size.y),1};

    vkCmdCopyBufferToImage(commandBuffer,buffer,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue,1,&submitInfo,VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device,commandPool,1,&commandBuffer);
}