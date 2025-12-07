#pragma once
#include "../../lib/glad/glad.h"
#include "../../lib/GLFW/glfw3.h"
#include "openglcommon.h"
#include "../vao.h"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../utils.h"

#ifdef CONVERT
#include "../../lib/shaders/glslang/SPIRV/GlslangToSpv.h"
#include "../../lib/shaders/glslang/glslang/Public/ShaderLang.h"
#include "../../lib/shaders/glslang/glslang/Public/ResourceLimits.h"

#include "../../lib/shaders/SPIRV-Cross/spirv_cross.hpp"
#include "../../lib/shaders/SPIRV-Cross/spirv_glsl.hpp"
#endif


class Shader{
private:
    static std::unordered_map<std::string,int> extractUniforms(GLuint program);
    static GLuint compileShader(GLenum type,std::string source);//me when only references are passed
    static GLuint createShaderProgram(std::string vertSource,std::string fragSource);
    void createShader();
    GLuint program=0;
    GLuint vaoIndex=0;
    std::unordered_map<int,GLint> textureLocs{};
    
    GLuint getProgram(){
        if(loaded&&!program)createShader();
        return program;
    }

    struct ShaderQueue{//only needed on windows cuz windows sux
        std::string vname;//won't act like macos doesn't also though
        std::string fname;//but it's easier to dev on it
        std::string vpath;
        std::string fpath;
    };
    ShaderQueue queue{};

    friend class Bento;
public:
    Shader(std::string vertPath, std::string fragPath,VAO vao);
    Shader(GLuint prgm,std::string vS,std::string fS,VAO v){
        program = prgm;
        vertSource = vS;
        fragSource = fS;
        vao = v;

        glGenVertexArrays(1,&vaoIndex);

        glBindVertexArray(vaoIndex);
        for(int i = 0; i < vao.attributes.size(); i++){
            GLuint format = GL_FLOAT;
            switch(vao.attributes[i].format){
                case AttribFormat::Float:format=GL_FLOAT;break;
                case AttribFormat::Int:format=GL_INT;break;
                case AttribFormat::UInt:format=GL_UNSIGNED_INT;break;
                case AttribFormat::Short:format=GL_SHORT;break;
            }

            if(buffers.size()<=i){
                buffers.resize(i+1);
                glGenBuffers(1,&buffers[i]);
            }
            glBindBuffer(GL_ARRAY_BUFFER,buffers[i]);
            glVertexAttribPointer(i,vao.attributes[i].size,format,GL_FALSE,0,(void*)(intptr_t)vao.attributes[i].offset);
            glEnableVertexAttribArray(i);
        }
    }
    ~Shader();

    std::string getUni();

    std::string vertSource = "";
    std::string fragSource = "";
    VAO vao{};

    #ifdef CONVERT
    static std::string convertShader(std::string source,EShLanguage lang,std::string name = "");
    #endif
};
