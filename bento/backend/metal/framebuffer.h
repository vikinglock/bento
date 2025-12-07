#pragma once

#include "../../lib/glm/glm.hpp"
#include <Metal/Metal.h>
#include <string>
#include <vector>

#include "texture.h"

class FrameBuffer{
private:
    MTLRenderPassDescriptor* passDescriptor;
    friend class Bento;//honestly couldn't care less it breaks encapsulation
    FrameBuffer(Texture* out){
        targets.push_back(out);
        attachments.push_back(0);
        size=glm::ivec2(-1);        
    }
    
public:
    std::vector<Texture*> targets;
    std::vector<uint> attachments;
    glm::ivec2 size;

    Texture* depthTexture;
    bool depthEnabled = false;

    FrameBuffer(){}

    FrameBuffer(Texture** textures,int* attachmnts,int count,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1)){
        setRenderTargets(textures,attachmnts,count,depthEnabled,s);
    }
    FrameBuffer(std::vector<Texture*> textures,std::vector<int> attachmnts,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1)){
        setRenderTargets(textures,attachmnts,depthEnabled,s);
    }

    void setRenderTargets(std::vector<Texture*> textures,std::vector<int> attachmnts,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1)){
        setRenderTargets(textures.data(),attachmnts.data(),textures.size(),depthEnabled,s);
    }
    void setRenderTargets(Texture** textures,int* attachmnts,int count,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1)){
        size = s;
        targets.resize(count);
        attachments.resize(count);
        if(!passDescriptor)passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        for(int i = 0; i < count; i++){
            targets[i] = textures[i];
            //if(!targets[i])targets[i]=new Texture(nullptr,glm::abs(size));
            attachments[i] = attachmnts[i];
            passDescriptor.colorAttachments[attachments[i]].texture = nil;//(__bridge id<MTLTexture>)targets[i]->getTexture();
            passDescriptor.colorAttachments[attachments[i]].clearColor = MTLClearColorMake(0,0,0,0.1);
            passDescriptor.colorAttachments[attachments[i]].loadAction = MTLLoadActionClear;
            passDescriptor.colorAttachments[attachments[i]].storeAction = MTLStoreActionStore;
        }
        this->depthEnabled = depthEnabled;

        if(depthEnabled){
            depthTexture = new Texture(nullptr,size==glm::ivec2(-1)?targets[0]->size:size,WrapMode::Repeat,MinFilter::Nearest,MagFilter::Nearest,glm::vec4(0.0f),TextureType::d2D,TextureFormat::D32Unorm);
            passDescriptor.depthAttachment.texture = nil;
            passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
            passDescriptor.depthAttachment.clearDepth = 1.0;
            passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        }
    }
    void resize(glm::ivec2 s){
        size = s;
        for(int i = 0; i < targets.size(); i++){
            targets[i]->setTexture(size);
            passDescriptor.colorAttachments[attachments[i]].texture = (__bridge id<MTLTexture>)targets[i]->getTexture();
        }
        if(depthEnabled){
            depthTexture->setTexture(size);
            passDescriptor.depthAttachment.texture = (__bridge id<MTLTexture>)depthTexture->getTexture();
        }
    }

    ~FrameBuffer(){
        [passDescriptor release];
        for(int i = 0; i < targets.size(); i++)
            if(targets[i])delete targets[i];
    }
};