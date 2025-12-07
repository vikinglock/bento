#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"
#include <iostream>
#include "../../utils.h"
#include "openglcommon.h"
#include "opengl.h"

#include <limits.h>
#include <string>

#ifdef MACOS
#include <unistd.h>
#include <mach-o/dyld.h>
#elif WINDOWS
#include <windows.h>
#endif


Texture::Texture(File file,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,glm::vec4 borderColor,TextureType texType,TextureFormat texFormat,bool mipmapped){
    createTexture(file.getData(),texType,texFormat,mipmapped,wrap,minFilter,magFilter,borderColor);
}

Texture::Texture(std::string path,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,glm::vec4 borderColor,TextureType texType,TextureFormat texFormat,bool mipmapped){
    std::vector<uint8_t> file = loadFile(path);
    createTexture(&file,texType,texFormat,mipmapped,wrap,minFilter,magFilter,borderColor);
}

Texture::Texture(void* data,glm::ivec2 size,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,glm::vec4 borderColor,TextureType texType,TextureFormat texFormat,bool mipmapped){
    if(!queueTex.loaded){
        queueTex.data = data;
        queueTex.size = size;
        queueTex.texType = texType;
        queueTex.texFormat = texFormat;
        queueTex.wrap = wrap;
        queueTex.minFilter = minFilter;
        queueTex.magFilter = magFilter;
        queueTex.borderColor = borderColor;
        queueTex.loaded = true;
    }
    if(loaded){
        glGenTextures(1,&texture);
        setTexture(queueTex.size,queueTex.texType,queueTex.texFormat);
        if(queueTex.data)setData(queueTex.data);
        changeSampler(queueTex.wrap,queueTex.minFilter,queueTex.magFilter,queueTex.borderColor);
    }
}

Texture::Texture() {}

void Texture::createTexture(std::vector<uint8_t>* data,TextureType texType,TextureFormat texFormat,bool mipmapped,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,glm::vec4 borderColor){
    if(!queueTex.loaded){
        int channels;
        int width,height;
        unsigned char* dat = stbi_load_from_memory(data->data(),data->size(),&width,&height,&channels,STBI_rgb_alpha);
        queueTex.data = dat;
        queueTex.size = {width,height};
        queueTex.texType = texType;
        queueTex.texFormat = texFormat;
        queueTex.wrap = wrap;
        queueTex.minFilter = minFilter;
        queueTex.magFilter = magFilter;
        queueTex.borderColor = borderColor;
        queueTex.loaded = true;
    }
    if(loaded){
        glGenTextures(1,&texture);
        setTexture(queueTex.size,queueTex.texType,queueTex.texFormat);
        if(queueTex.data)setData(queueTex.data);
        changeSampler(queueTex.wrap,queueTex.minFilter,queueTex.magFilter,queueTex.borderColor);
    }
}

void Texture::changeSampler(WrapMode wrap,MinFilter minFilter,MagFilter magFilter,glm::vec4 borderColor) {
    if(samplerCreated&&wrap==currentWrap&&minFilter==currentMinFilter&&magFilter==currentMagFilter&&borderColor==currentBorderColor)return;
    samplerCreated = true;

    glBindTexture((GLuint)texType,texture);
    GLint glMinFilter = GL_NEAREST;
    switch(minFilter){
        case MinFilter::Nearest:      glMinFilter=GL_NEAREST;break;
        case MinFilter::Linear:       glMinFilter=GL_LINEAR;break;
        case MinFilter::NearestMipmap:glMinFilter=GL_NEAREST_MIPMAP_NEAREST;break;
        case MinFilter::LinearMipmap: glMinFilter=GL_LINEAR_MIPMAP_LINEAR;break;
    }
    GLint glMagFilter = (magFilter==MagFilter::Linear)?GL_LINEAR:GL_NEAREST;
    GLint glWrap = GL_REPEAT;
    switch(wrap){
        case WrapMode::Repeat:        glWrap=GL_REPEAT;break;
        case WrapMode::MirroredRepeat:glWrap=GL_MIRRORED_REPEAT;break;
        case WrapMode::ClampToEdge:   glWrap=GL_CLAMP_TO_EDGE;break;
        case WrapMode::ClampToBorder: glWrap=GL_CLAMP_TO_BORDER;break;
    }
    glTexParameteri((GLuint)texType,GL_TEXTURE_MIN_FILTER,glMinFilter);
    glTexParameteri((GLuint)texType,GL_TEXTURE_MAG_FILTER,glMagFilter);
    glTexParameteri((GLuint)texType,GL_TEXTURE_WRAP_S,glWrap);
    glTexParameteri((GLuint)texType,GL_TEXTURE_WRAP_T,glWrap);
    if(wrap==WrapMode::ClampToBorder)
        glTexParameterfv((GLuint)texType, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);

    currentWrap = wrap;
    currentMinFilter = minFilter;
    currentMagFilter = magFilter;
    currentBorderColor = borderColor;
}

void Texture::setTexture(glm::ivec2 size,TextureType texType,TextureFormat texFormat,bool mipmapped){
    glBindTexture((GLuint)texType,texture);
    
    GLuint fmt;
    GLuint tp;
    switch(texFormat){
        case TextureFormat::A8Unorm:    fmt=GL_ALPHA;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::R8Unorm:    fmt=GL_RED;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::R8Snorm:    fmt=GL_RED;tp=GL_BYTE;break;
        case TextureFormat::R8Uint:     fmt=GL_RED_INTEGER;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::R8Sint:     fmt=GL_RED_INTEGER;tp=GL_BYTE;break;
        case TextureFormat::RG8Unorm:   fmt=GL_RG;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::RG8Snorm:   fmt=GL_RG;tp=GL_BYTE;break;
        case TextureFormat::RG8Uint:    fmt=GL_RG_INTEGER;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::RG8Sint:    fmt=GL_RG_INTEGER;tp=GL_BYTE;break;
        case TextureFormat::RGBA8Unorm: fmt=GL_RGBA;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::RGBA8col:   fmt=GL_RGBA;tp=GL_BYTE;break;
        case TextureFormat::RGBA8Snorm: fmt=GL_RGBA;tp=GL_UNSIGNED_BYTE;break;
        case TextureFormat::RGBA8Uint:  fmt=GL_RGBA_INTEGER;tp=GL_BYTE;break;
        case TextureFormat::RGBA8Sint:  fmt=GL_RGBA_INTEGER;tp=GL_UNSIGNED_BYTE;break;

        case TextureFormat::R16Unorm:   fmt=GL_RED;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::R16Snorm:   fmt=GL_RED;tp=GL_SHORT;break;
        case TextureFormat::R16Uint:    fmt=GL_RED_INTEGER;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::R16Sint:    fmt=GL_RED_INTEGER;tp=GL_SHORT;break;
        case TextureFormat::R16Float:   fmt=GL_RED;tp=GL_HALF_FLOAT;break;
        case TextureFormat::RG16Unorm:  fmt=GL_RG;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::RG16Snorm:  fmt=GL_RG;tp=GL_SHORT;break;
        case TextureFormat::RG16Uint:   fmt=GL_RG_INTEGER;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::RG16Sint:   fmt=GL_RG_INTEGER;tp=GL_SHORT;break;
        case TextureFormat::RG16Float:  fmt=GL_RG;tp=GL_HALF_FLOAT;break;
        case TextureFormat::RGBA16Unorm:    fmt=GL_RGBA;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::RGBA16Snorm:    fmt=GL_RGBA;tp=GL_SHORT;break;
        case TextureFormat::RGBA16Uint:     fmt=GL_RGBA_INTEGER;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::RGBA16Sint:     fmt=GL_RGBA_INTEGER;tp=GL_SHORT;break;
        case TextureFormat::RGBA16Float:    fmt=GL_RGBA;tp=GL_HALF_FLOAT;break;

        case TextureFormat::R32Sint:    fmt=GL_RED_INTEGER;tp=GL_INT;break;
        case TextureFormat::R32Uint:    fmt=GL_RED_INTEGER;tp=GL_INT;break;
        case TextureFormat::R32Float:   fmt=GL_RED;tp=GL_FLOAT;break;
        case TextureFormat::RG32Uint:   fmt=GL_RG_INTEGER;tp=GL_INT;break;
        case TextureFormat::RG32Sint:   fmt=GL_RG_INTEGER;tp=GL_INT;break;
        case TextureFormat::RG32Float:  fmt=GL_RG;tp=GL_FLOAT;break;
        case TextureFormat::RGBA32Uint:     fmt=GL_RGBA_INTEGER;tp=GL_INT;break;
        case TextureFormat::RGBA32Sint:     fmt=GL_RGBA_INTEGER;tp=GL_INT;break;
        case TextureFormat::RGBA32Float:    fmt=GL_RGBA;tp=GL_FLOAT;break;


        case TextureFormat::D16Unorm:   fmt=GL_DEPTH_COMPONENT;tp=GL_UNSIGNED_SHORT;break;
        case TextureFormat::D24Unorm:   fmt=GL_DEPTH_COMPONENT;tp=GL_UNSIGNED_INT;break;
        case TextureFormat::D32Unorm:   fmt=GL_DEPTH_COMPONENT;tp=GL_UNSIGNED_INT;break;
        case TextureFormat::D32Float:   fmt=GL_DEPTH_COMPONENT;tp=GL_UNSIGNED_SHORT;break;

        case TextureFormat::D24S8:      fmt=GL_DEPTH_STENCIL;tp=GL_UNSIGNED_INT_24_8;break;
        case TextureFormat::D32S8:      fmt=GL_DEPTH_STENCIL;tp=GL_FLOAT_32_UNSIGNED_INT_24_8_REV;break;
    };
    
    glTexImage2D((GLuint)texType,0,(GLint)texFormat,size.x,size.y,0,fmt,tp,nullptr);
    this->size = size;
    this->texFormat = texFormat;
    this->texType = texType;
    this->mipmapped = mipmapped;
}

void* Texture::getData(){
    glBindTexture((GLuint)texType,texture);
    float* pixels = new float[size.x*size.y*4];
    glGetTexImage((GLuint)texType,0,GL_RGBA,GL_FLOAT,pixels);
    return pixels;
}

void Texture::setData(void* data){
    glBindTexture((GLuint)texType,texture);
    glTexSubImage2D((GLuint)texType,0,0,0,size.x,size.y,GL_RGBA,GL_UNSIGNED_BYTE,data);
}

void Texture::genMipmaps(){
    glBindTexture((GLuint)texType,texture);
    glGenerateMipmap((GLuint)texType);
}

Texture::Texture(unsigned int tex){
    texture = tex;
    texType = TextureType::d2D;
}

Texture::~Texture(){
    if(!texture)glDeleteTextures(1, &texture);
}


unsigned int Texture::getTexture(){
    if(!texture)createTexture(nullptr);
    if(!texture)std::cerr<<"couldn't create texture, suggested fix: do not access texture outside of int main().\ni don't know why opengl is like this"<<std::endl;
    return texture;
}
