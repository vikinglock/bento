#pragma once

#include "../../lib/glm/glm.hpp"
#include <string>
#include <vector>
#include "../../lib/glad/glad.h"
#include "../../lib/GLFW/glfw3.h"

#include "texture.h"


class FrameBuffer{
private:
    GLuint fbo = 0;
    friend class Bento;//honestly couldn't care less it breaks encapsulation
    FrameBuffer(Texture* out){
        targets.push_back(out);
        attachments.push_back(GL_COLOR_ATTACHMENT0);
        size=glm::ivec2(-1);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,out->getTexture(),0);
    }
    GLuint getFBO(){
        if(!fbo){
            glGenFramebuffers(1,&fbo);
            glBindFramebuffer(GL_FRAMEBUFFER,fbo);
            
            for(int i = 0; i < targets.size(); i++)
                glFramebufferTexture2D(GL_FRAMEBUFFER,attachments[i],GL_TEXTURE_2D,targets[i]->getTexture(),0);
            if(depthEnabled){
                depthTexture = new Texture(nullptr,size==glm::ivec2(-1)?targets[0]->size:size,WrapMode::Repeat,MinFilter::Nearest,MagFilter::Nearest,glm::vec4(0.0f),TextureType::d2D,TextureFormat::D32Unorm);
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthTexture->getTexture(),0);
            }
        }
        return fbo;
    }
    
public:
    std::vector<Texture*> targets{};
    std::vector<GLuint> attachments{};
    glm::ivec2 size{};

    Texture* depthTexture;
    bool depthEnabled = false;

    FrameBuffer(){}

    FrameBuffer(Texture** textures,int* attachmnts,int count,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1));
    FrameBuffer(std::vector<Texture*> textures,std::vector<int> attachmnts,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1));
    void setRenderTargets(std::vector<Texture*> textures,std::vector<int> attachmnts,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1));
    void setRenderTargets(Texture** textures,int* attachmnts,int count,bool depthEnabled = true,glm::ivec2 s = glm::ivec2(-1));

    void resize(glm::ivec2 s);
    
    ~FrameBuffer();
};