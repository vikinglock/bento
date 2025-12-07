#include "framebuffer.h"

FrameBuffer::FrameBuffer(Texture** textures,int* attachmnts,int count,bool depthEnabled,glm::ivec2 s){
    setRenderTargets(textures,attachmnts,count,depthEnabled,s);
}
FrameBuffer::FrameBuffer(std::vector<Texture*> textures,std::vector<int> attachmnts,bool depthEnabled,glm::ivec2 s){
    setRenderTargets(textures,attachmnts,depthEnabled,s);
}

void FrameBuffer::setRenderTargets(std::vector<Texture*> textures,std::vector<int> attachmnts,bool depthEnabled,glm::ivec2 s){
    setRenderTargets(textures.data(),attachmnts.data(),textures.size(),depthEnabled,s);
}
void FrameBuffer::setRenderTargets(Texture** textures,int* attachmnts,int count,bool depthEnabled,glm::ivec2 s){
    size = s;
    targets.resize(count);
    attachments.resize(count);
    for(int i = 0; i < count; i++){
        //targets[i]->setTexture(size);
        targets[i] = textures[i];
        attachments[i] = GL_COLOR_ATTACHMENT0+attachmnts[i];
    }
    this->depthEnabled = depthEnabled;
}
void FrameBuffer::resize(glm::ivec2 s){
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    size = s;
    for(int i = 0; i < targets.size(); i++){
        targets[i]->setTexture(size);
    }
    if(depthEnabled){
        depthTexture->setTexture(size,TextureType::d2D,TextureFormat::D32S8);
    }
}

FrameBuffer::~FrameBuffer(){
    glDeleteFramebuffers(1,&fbo);
    for(int i = 0; i < targets.size(); i++)
        if(targets[i])delete targets[i];
}