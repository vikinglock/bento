#pragma once

#include "../lib/glm/glm.hpp"
#include <string>
#include "vulkancommon.h"
#include "../file/file.h"

class Texture;
#include "vulkan.h"

enum class MinFilter {
    Nearest,
    Linear,
    NearestMipmap,
    LinearMipmap
};

enum class MagFilter {
    Nearest,
    Linear
};

enum class WrapMode {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

enum TextureType {
    textureType1D,
    textureType1DArray,
    textureType2D,
    textureType2DArray,
    textureType2DMulti,
    textureTypeCubeMap,
    textureTypeCubeMapArray,
    textureType3D,
    textureTypeBuffer
};

class Texture{
public:
    Texture();
    Texture(File file,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f),TextureType texType=textureType2D);
    Texture(std::string filepath,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f),TextureType texType=textureType2D);
    Texture(unsigned int texture);
    Texture(float* data,glm::ivec2 size,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f),TextureType texType=textureType2D);
    ~Texture();

    unsigned int getTexture();
    void setTexture(glm::ivec2 size,TextureType texType=textureType2D,bool mipmapped=true);

    void* getData();
    void setData(void* data);

    void changeSampler(WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f));
    void genMipmaps();

    void transitionImageLayout(VkImage image,VkFormat format,VkImageLayout oldLayout,VkImageLayout newLayout);
    
    
    glm::ivec2 size;
    
    bool mipmapped;
    WrapMode currentWrap;
    MinFilter currentMinFilter;
    MagFilter currentMagFilter;
    glm::vec4 currentBorderColor;
    TextureType texType;
private:
    void copyBufferToImage(VkBuffer buffer,VkImage image,uint32_t width,uint32_t height);
    
    std::string filepath;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkImage texture;
    VkDeviceMemory textureMemory;
    VkImageView imageView;
    VkSampler sampler;

    int channels;
};