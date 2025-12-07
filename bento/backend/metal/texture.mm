#import <CoreGraphics/CoreGraphics.h>
#import <Metal/Metal.h>
#include "../../utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"

#include "texture.h"
#include <iostream>


#include <unistd.h>
#include <limits.h>
#include <string>
#include <mach-o/dyld.h>

void Texture::createTexture(std::vector<uint8_t>* data,TextureType texType,TextureFormat texFormat,bool mipmapped){
    int channels;
    int width,height;
    unsigned char* dat = stbi_load_from_memory(data->data(),data->size(),&width,&height,&channels,STBI_rgb_alpha);
    size = {width,height};

    /*
    CGImageSourceRef source = CGImageSourceCreateWithData((__bridge CFDataRef)[NSData dataWithBytes:data->data() length:data->size()], NULL);
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CFRelease(source);

    width = CGImageGetWidth(image);
    height = CGImageGetHeight(image);

    std::vector<uint8_t> pixels(width*height*4);
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(pixels.data(),width,height,8,width*4,cs,(CGBitmapInfo)kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);//js reference

    //CGContextTranslateCTM(context, 0, height);
    //CGContextScaleCTM(context, 1.0, -1.0);

    CGContextDrawImage(ctx,CGRectMake(0,0,width,height),image);
    CGContextRelease(ctx);
    CGColorSpaceRelease(cs);
    CGImageRelease(image);
    */

    setTexture(size,texType,texFormat);
    setData(dat);
}

Texture::Texture(File file,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,const glm::vec4& borderColor,TextureType texType,TextureFormat texFormat,bool mipmapped){
    changeSampler(wrap,minFilter,magFilter,borderColor);
    createTexture(file.getData(),texType,texFormat,mipmapped);
}

Texture::Texture(std::string path,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,const glm::vec4& borderColor,TextureType texType,TextureFormat texFormat,bool mipmapped){
    std::vector<uint8_t> file = loadFile(path);
    changeSampler(wrap,minFilter,magFilter,borderColor);
    createTexture(&file,texType,texFormat,mipmapped);
}

Texture::Texture(){
    texture = nil;
    sampler = nil;
}

Texture::Texture(void* data,glm::ivec2 size,WrapMode wrap,MinFilter minFilter,MagFilter magFilter,const glm::vec4& borderColor,TextureType texType,TextureFormat texFormat,bool mipmapped){
    changeSampler(wrap,minFilter,magFilter,borderColor);
    setTexture(size,texType,texFormat);
    if(data)setData(data);
}

void Texture::changeSampler(WrapMode wrap,MinFilter minFilter,MagFilter magFilter,const glm::vec4& borderColor){
    if(wrap==currentWrap&&minFilter==currentMinFilter&&magFilter==currentMagFilter&&texture)return;

    MTLSamplerDescriptor* desc = [[MTLSamplerDescriptor alloc] init];

    switch(minFilter){
        case MinFilter::Nearest:
            desc.minFilter = MTLSamplerMinMagFilterNearest;
            desc.mipFilter = MTLSamplerMipFilterNotMipmapped;
            break;
        case MinFilter::Linear:
            desc.minFilter = MTLSamplerMinMagFilterLinear;
            desc.mipFilter = MTLSamplerMipFilterNotMipmapped;
            break;
        case MinFilter::NearestMipmap:
            desc.minFilter = MTLSamplerMinMagFilterNearest;
            desc.mipFilter = MTLSamplerMipFilterNearest;
            break;
        case MinFilter::LinearMipmap:
            desc.minFilter = MTLSamplerMinMagFilterLinear;
            desc.mipFilter = MTLSamplerMipFilterLinear;
            break;
    }

    switch(magFilter){
        case MagFilter::Nearest: desc.magFilter = MTLSamplerMinMagFilterNearest; break;
        case MagFilter::Linear:  desc.magFilter = MTLSamplerMinMagFilterLinear;  break;
    }

    MTLSamplerAddressMode addressMode;
    switch(wrap){
        case WrapMode::Repeat:          addressMode = MTLSamplerAddressModeRepeat; break;
        case WrapMode::MirroredRepeat:  addressMode = MTLSamplerAddressModeMirrorRepeat; break;
        case WrapMode::ClampToEdge:     addressMode = MTLSamplerAddressModeClampToEdge; break;
        case WrapMode::ClampToBorder:   addressMode = MTLSamplerAddressModeClampToZero; break;
    }


    desc.lodMinClamp = 0.0;
    desc.lodMaxClamp = texture.mipmapLevelCount - 1;

    desc.sAddressMode = addressMode;
    desc.tAddressMode = addressMode;
    desc.rAddressMode = addressMode;

    sampler = [device newSamplerStateWithDescriptor:desc];

    currentWrap = wrap;
    currentMinFilter = minFilter;
    currentMagFilter = magFilter;
}

void Texture::genMipmaps(){
    if(!texture||texture.mipmapLevelCount <= 1)return;
    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];

    [blitEncoder generateMipmapsForTexture:texture];

    [blitEncoder endEncoding];

    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

Texture::~Texture(){if(texture)[texture release];}

void* Texture::getTexture(){return (__bridge void*)texture;}

void Texture::setTexture(glm::ivec2 size,TextureType texType,TextureFormat texFormat,bool mipmapped){
    @autoreleasepool{
        MTLPixelFormat format;

        switch(texFormat){
            case TextureFormat::A8Unorm:        format = MTLPixelFormatA8Unorm;break;
            case TextureFormat::R8Unorm:        format = MTLPixelFormatR8Unorm;break;
            case TextureFormat::R8col:          format = MTLPixelFormatR8Unorm_sRGB;break;
            case TextureFormat::R8Snorm:        format = MTLPixelFormatR8Snorm;break;
            case TextureFormat::R8Uint:         format = MTLPixelFormatR8Uint;break;
            case TextureFormat::R8Sint:         format = MTLPixelFormatR8Sint;break;
            case TextureFormat::RG8Unorm:       format = MTLPixelFormatRG8Unorm;break;
            case TextureFormat::RG8col:         format = MTLPixelFormatRG8Unorm_sRGB;break;
            case TextureFormat::RG8Snorm:       format = MTLPixelFormatRG8Snorm;break;
            case TextureFormat::RG8Uint:        format = MTLPixelFormatRG8Uint;break;
            case TextureFormat::RG8Sint:        format = MTLPixelFormatRG8Sint;break;
            case TextureFormat::RGBA8Unorm:     format = MTLPixelFormatRGBA8Unorm;break;
            case TextureFormat::RGBA8col:       format = MTLPixelFormatRGBA8Unorm_sRGB;break;
            case TextureFormat::RGBA8Snorm:     format = MTLPixelFormatRGBA8Snorm;break;
            case TextureFormat::RGBA8Uint:      format = MTLPixelFormatRGBA8Uint;break;
            case TextureFormat::RGBA8Sint:      format = MTLPixelFormatRGBA8Sint;break;

            case TextureFormat::R16Unorm:       format = MTLPixelFormatR16Unorm;break;
            case TextureFormat::R16Snorm:       format = MTLPixelFormatR16Snorm;break;
            case TextureFormat::R16Uint:        format = MTLPixelFormatR16Uint;break;
            case TextureFormat::R16Sint:        format = MTLPixelFormatR16Sint;break;
            case TextureFormat::R16Float:       format = MTLPixelFormatR16Float;break;
            case TextureFormat::RG16Unorm:      format = MTLPixelFormatRG16Unorm;break;
            case TextureFormat::RG16Snorm:      format = MTLPixelFormatRG16Snorm;break;
            case TextureFormat::RG16Uint:       format = MTLPixelFormatRG16Uint;break;
            case TextureFormat::RG16Sint:       format = MTLPixelFormatRG16Sint;break;
            case TextureFormat::RG16Float:      format = MTLPixelFormatRG16Float;break;
            case TextureFormat::RGBA16Unorm:    format = MTLPixelFormatRGBA16Unorm;break;
            case TextureFormat::RGBA16Snorm:    format = MTLPixelFormatRGBA16Snorm;break;
            case TextureFormat::RGBA16Uint:     format = MTLPixelFormatRGBA16Uint;break;
            case TextureFormat::RGBA16Sint:     format = MTLPixelFormatRGBA16Sint;break;
            case TextureFormat::RGBA16Float:    format = MTLPixelFormatRGBA16Float;break;

            case TextureFormat::R32Uint:        format = MTLPixelFormatR32Uint;break;
            case TextureFormat::R32Sint:        format = MTLPixelFormatR32Sint;break;
            case TextureFormat::R32Float:       format = MTLPixelFormatR32Float;break;
            case TextureFormat::RG32Uint:       format = MTLPixelFormatRG32Uint;break;
            case TextureFormat::RG32Sint:       format = MTLPixelFormatRG32Sint;break;
            case TextureFormat::RG32Float:      format = MTLPixelFormatRG32Float;break;
            case TextureFormat::RGBA32Uint:     format = MTLPixelFormatRGBA32Uint;break;
            case TextureFormat::RGBA32Sint:     format = MTLPixelFormatRGBA32Sint;break;
            case TextureFormat::RGBA32Float:    format = MTLPixelFormatRGBA32Float;break;


            case TextureFormat::D16Unorm:       format = MTLPixelFormatDepth16Unorm;break;
            case TextureFormat::D24Unorm:       format = MTLPixelFormatDepth32Float;break;
            case TextureFormat::D32Unorm:       format = MTLPixelFormatDepth32Float;break;//?
            case TextureFormat::D32Float:       format = MTLPixelFormatDepth32Float;break;

            case TextureFormat::D24S8:          format = MTLPixelFormatDepth24Unorm_Stencil8;break;
            case TextureFormat::D32S8:          format = MTLPixelFormatDepth32Float_Stencil8;break;

        }


        MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:size.x height:size.y mipmapped:mipmapped];
        desc.usage = MTLTextureUsageRenderTarget|MTLTextureUsageShaderRead;
        desc.storageMode = MTLStorageModeShared;

        switch(texType){
            case TextureType::d1D:desc.textureType = MTLTextureType1D;break;
            case TextureType::d1DArray:desc.textureType = MTLTextureType1DArray;break;
            case TextureType::d2D:desc.textureType = MTLTextureType2D;break;
            case TextureType::d2DArray:desc.textureType = MTLTextureType2DArray;break;
            case TextureType::d2DMulti:desc.textureType = MTLTextureType2DMultisample;break;
            case TextureType::CubeMap:desc.textureType = MTLTextureTypeCube;break;
            case TextureType::CubeMapArray:desc.textureType = MTLTextureTypeCubeArray;break;
            case TextureType::d3D:desc.textureType = MTLTextureType3D;break;
            case TextureType::Buffer:desc.textureType = MTLTextureTypeTextureBuffer;break;
        }
        if(texture)[texture release];

        texture = [device newTextureWithDescriptor:desc];

        this->size = size;
        this->mipmapped = mipmapped;
        this->texType = texType;
        this->texFormat = texFormat;
    }
}

void* Texture::getData(){
    void* data = malloc(size.x*size.y);
    [texture getBytes:data bytesPerRow:texture.width*4 fromRegion:MTLRegionMake2D(0,0,size.x,size.y) mipmapLevel:0];//why is this backwards
    return data;//                                                                                                   is it like code they made a while apart so they forgot or something
}

void Texture::setData(void* data){
    [texture replaceRegion:MTLRegionMake2D(0,0,size.x,size.y) mipmapLevel:0 withBytes:data bytesPerRow:size.x*4];
}