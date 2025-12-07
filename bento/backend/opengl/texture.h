#pragma once

#include "../../lib/glm/glm.hpp"
#include "../../lib/glad/glad.h"
#include "../../lib/GLFW/glfw3.h"
#include <string>
#include "../file/file.h"

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

enum class TextureType {
    d1D = GL_TEXTURE_1D,
    d1DArray = GL_TEXTURE_1D_ARRAY,
    d2D = GL_TEXTURE_2D,
    d2DArray = GL_TEXTURE_2D_ARRAY,
    d2DMulti = GL_TEXTURE_2D_MULTISAMPLE,
    CubeMap = GL_TEXTURE_CUBE_MAP,
    CubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
    d3D = GL_TEXTURE_3D,
    Buffer = GL_TEXTURE_BUFFER
};

enum class TextureFormat {
    A8Unorm=GL_ALPHA8,
    R8Unorm=      GL_R8,R8col=             GL_R8,R8Snorm=      GL_R8_SNORM,R8Uint=      GL_R8UI,R8Sint=      GL_R8I,
    RG8Unorm=    GL_RG8,RG8col=           GL_RG8,RG8Snorm=    GL_RG8_SNORM,RG8Uint=    GL_RG8UI,RG8Sint=    GL_RG8I,
    RGBA8Unorm=GL_RGBA8,RGBA8col=GL_SRGB8_ALPHA8,RGBA8Snorm=GL_RGBA8_SNORM,RGBA8Uint=GL_RGBA8UI,RGBA8Sint=GL_RGBA8I,

    R16Unorm=      GL_R16,R16Snorm=      GL_R16_SNORM,R16Uint=      GL_R16UI,R16Sint=      GL_R16I,R16Float=      GL_R16F,
    RG16Unorm=    GL_RG16,RG16Snorm=    GL_RG16_SNORM,RG16Uint=    GL_RG16UI,RG16Sint=    GL_RG16I,RG16Float=    GL_RG16F,
    RGBA16Unorm=GL_RGBA16,RGBA16Snorm=GL_RGBA16_SNORM,RGBA16Uint=GL_RGBA16UI,RGBA16Sint=GL_RGBA16I,RGBA16Float=GL_RGBA16F,

    R32Uint=      GL_R32UI,R32Sint=      GL_R32I,R32Float=      GL_R32F,
    RG32Uint=    GL_RG32UI,RG32Sint=    GL_RG32I,RG32Float=    GL_RG32F,
    RGBA32Uint=GL_RGBA32UI,RGBA32Sint=GL_RGBA32I,RGBA32Float=GL_RGBA32F,

    D16Unorm=GL_DEPTH_COMPONENT16,
    D24Unorm=GL_DEPTH_COMPONENT24,
    D32Unorm=GL_DEPTH_COMPONENT32,
    D32Float=GL_DEPTH_COMPONENT32F,

    D24S8=GL_DEPTH24_STENCIL8,
    D32S8=GL_DEPTH32F_STENCIL8,
};

class Texture {
private:
    std::string filepath;

    unsigned int texture = 0;
    int channels;
    bool samplerCreated = false;

    struct TextureQueue{
        WrapMode wrap;
        MinFilter minFilter;
        MagFilter magFilter;
        glm::vec4 borderColor;
        
        TextureType texType;
        TextureFormat texFormat;
        bool mipmapped;
        bool loaded;
        void* data;
        glm::ivec2 size;
    };

    TextureQueue queueTex{};//cuz opengl kinda sucks

    void createTexture(std::vector<uint8_t>* data,TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,glm::vec4 borderColor=glm::vec4(0.0f));
public:
    Texture();
    Texture(File file,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,glm::vec4 borderColor=glm::vec4(0.0f),TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
    Texture(std::string filepath,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,glm::vec4 borderColor=glm::vec4(0.0f),TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
    Texture(unsigned int texture);
    Texture(void* data,glm::ivec2 size,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,glm::vec4 borderColor=glm::vec4(0.0f),TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
    ~Texture();

    unsigned int getTexture();
    void setTexture(glm::ivec2 size,TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);

    void* getData();
    void setData(void* data);

    void changeSampler(WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,glm::vec4 borderColor=glm::vec4(0.0f));
    void genMipmaps();
    
    
    glm::ivec2 size;

    bool mipmapped;
    WrapMode currentWrap;
    MinFilter currentMinFilter;
    MagFilter currentMagFilter;
    glm::vec4 currentBorderColor;
    TextureType texType;
    TextureFormat texFormat;
};