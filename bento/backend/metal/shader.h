#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <regex>
#include <simd/simd.h>
#include <mach-o/dyld.h>
#include "../vao.h"
#include "metalcommon.h"
#include <Metal/Metal.h>

#ifdef CONVERT
#include "../../lib/shaders/glslang/SPIRV/GlslangToSpv.h"
#include "../../lib/shaders/glslang/glslang/Public/ShaderLang.h"
#include "../../lib/shaders/glslang/glslang/Public/ResourceLimits.h"

#include "../../lib/shaders/SPIRV-Cross/spirv_cross.hpp"
#include "../../lib/shaders/SPIRV-Cross/spirv_msl.hpp"
#endif


class Shader{
private:
    std::unordered_map<std::string, int> uniformMapVert;
    std::unordered_map<std::string, int> sizeMapVert;
    std::unordered_map<std::string, int> uniformMapFrag;
    std::unordered_map<std::string, int> sizeMapFrag;
    friend class Bento;
public:
    #ifdef CONVERT
    static std::string convertShader(std::string source,EShLanguage lang,std::string name = "");
    #endif

    Shader(std::string vertPath, std::string fragPath,VAO vao = VAO());
    Shader(id<MTLRenderPipelineState> plState,std::string vS,std::string fS,VAO vao = VAO());
    
    id<MTLRenderPipelineState> pipelineState;
    id<MTLBuffer> fragBuffer;
    id<MTLBuffer> vertBuffer;
    
    std::string getUni();
    
    std::string vertSource = "";
    std::string fragSource = "";
};