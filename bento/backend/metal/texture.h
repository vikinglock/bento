#pragma once

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#ifdef IOS
#include <UIKit/UIKit.h>
#else
#include <Cocoa/Cocoa.h>
#endif
#include "metalcommon.h"
#include "../../lib/glm/glm.hpp"
#include "../file/file.h"
#include <string>

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
    d1D,
    d1DArray,
    d2D,
    d2DArray,
    d2DMulti,
    CubeMap,
    CubeMapArray,
    d3D,
    Buffer
};

enum class TextureFormat {
    A8Unorm,
    R8Unorm,   R8col,   R8Snorm,   R8Uint,   R8Sint,
    RG8Unorm,  RG8col,  RG8Snorm,  RG8Uint,  RG8Sint,
    RGBA8Unorm,RGBA8col,RGBA8Snorm,RGBA8Uint,RGBA8Sint,

    R16Unorm,   R16Snorm,   R16Uint,   R16Sint,   R16Float,
    RG16Unorm,  RG16Snorm,  RG16Uint,  RG16Sint,  RG16Float,
    RGBA16Unorm,RGBA16Snorm,RGBA16Uint,RGBA16Sint,RGBA16Float,

    R32Uint,   R32Sint,   R32Float,
    RG32Uint,  RG32Sint,  RG32Float,
    RGBA32Uint,RGBA32Sint,RGBA32Float,//because 24 bits suck actually

    D16Unorm,D24Unorm,D32Unorm,D32Float,

    D24S8,D32S8,
};

/*
Depth and stencil pixel formats
MTLPixelFormatStencil8                      ///A pixel format with an 8-bit unsigned integer component, used for a stencil render target.
MTLPixelFormatX32_Stencil8                  ///A stencil pixel format used to read the stencil value from a texture with a combined 32-bit depth and 8-bit stencil value.
MTLPixelFormatX24_Stencil8                  ///A stencil pixel format used to read the stencil value from a texture with a combined 24-bit depth and 8-bit stencil value.

Packed 16-bit pixel formats
MTLPixelFormatB5G6R5Unorm                   ///Packed 16-bit format with normalized unsigned integer color components: 5 bits for blue, 6 bits for green, 5 bits for red, packed into 16 bits.
MTLPixelFormatA1BGR5Unorm                   ///Packed 16-bit format with normalized unsigned integer color components: 5 bits each for BGR and 1 for alpha, packed into 16 bits.
MTLPixelFormatABGR4Unorm                    ///Packed 16-bit format with normalized unsigned integer color components: 4 bits each for ABGR, packed into 16 bits.
MTLPixelFormatBGR5A1Unorm                   ///Packed 16-bit format with normalized unsigned integer color components: 5 bits each for BGR and 1 for alpha, packed into 16 bits.
Packed 32-bit pixel formats
MTLPixelFormatBGR10A2Unorm                  ///A 32-bit packed pixel format with four normalized unsigned integer components: 10-bit blue, 10-bit green, 10-bit red, and 2-bit alpha.
MTLPixelFormatRGB10A2Unorm                  ///A 32-bit packed pixel format with four normalized unsigned integer components: 10-bit red, 10-bit green, 10-bit blue, and 2-bit alpha.
MTLPixelFormatRGB10A2Uint                   ///A 32-bit packed pixel format with four unsigned integer components: 10-bit red, 10-bit green, 10-bit blue, and 2-bit alpha.
MTLPixelFormatRG11B10Float                  ///32-bit format with floating-point color components, 11 bits each for red and green and 10 bits for blue.
MTLPixelFormatRGB9E5Float                   ///Packed 32-bit format with floating-point color components: 9 bits each for RGB and 5 bits for an exponent shared by RGB, packed into 32 bits.

Extended range and wide color pixel formats
MTLPixelFormatBGRA10_XR                     ///A 64-bit extended-range pixel format with four fixed-point components of 10-bit blue, 10-bit green, 10-bit red, and 10-bit alpha.
MTLPixelFormatBGRA10_XR_sRGB                ///A 64-bit extended-range pixel format with sRGB conversion and four fixed-point components of 10-bit blue, 10-bit green, 10-bit red, and 10-bit alpha.
MTLPixelFormatBGR10_XR                      ///A 32-bit extended-range pixel format with three fixed-point components of 10-bit blue, 10-bit green, and 10-bit red.
MTLPixelFormatBGR10_XR_sRGB                 ///A 32-bit extended-range pixel format with sRGB conversion and three fixed-point components of 10-bit blue, 10-bit green, and 10-bit red.
*/

class Texture {//i touched (:
private:
    id<MTLTexture> texture;
    id<MTLSamplerState> sampler;
    friend class Bento;

    void createTexture(std::vector<uint8_t>* data,TextureType texType,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
public:
    Texture();
    Texture(File file,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f),TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
    Texture(std::string filepath,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f),TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
    Texture(unsigned int data);
    Texture(void* data,glm::ivec2 size,WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f),TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);
    ~Texture();
    
    void* getTexture();
    void setTexture(glm::ivec2 size,TextureType texType=TextureType::d2D,TextureFormat texFormat=TextureFormat::RGBA8Unorm,bool mipmapped=false);

    void* getData();
    void setData(void* data);

    void changeSampler(WrapMode wrap=WrapMode::Repeat,MinFilter minFilter=MinFilter::Nearest,MagFilter magFilter=MagFilter::Nearest,const glm::vec4& borderColor=glm::vec4(0.0f));
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
