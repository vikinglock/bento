#include "shader.h"
#include "../../utils.h"


Shader::Shader(std::string vertPath, std::string fragPath,VAO vao) {
    std::string dir = getExecutablePath();
    
    vertPath = "/"+vertPath;
    fragPath = "/"+fragPath;
    
    size_t lastSlash = vertPath.find_last_of("/\\");
    std::string vertDir = "";
    std::string vertFilename = "";
    if (lastSlash != std::string::npos) {
        vertDir = vertPath.substr(0, lastSlash);
        vertFilename = vertPath.substr(lastSlash, vertPath.rfind('.') - lastSlash);
    }
    lastSlash = fragPath.find_last_of("/\\");
    std::string fragDir = "";
    std::string fragFilename = "";
    if (lastSlash != std::string::npos) {
        fragDir = fragPath.substr(0, lastSlash);
        fragFilename = fragPath.substr(lastSlash, fragPath.rfind('.') - lastSlash);
    }
    NSError *error = nil;
    NSString *vertShaderSource;
    NSString *fragShaderSource;
    std::string vSource = loadFileString(dir+vertPath);
    std::string fSource = loadFileString(dir+fragPath);

    vertSource = Shader::convertShader(vSource,EShLangVertex,vertFilename);
    fragSource = Shader::convertShader(fSource,EShLangFragment,fragFilename);


    std::ofstream oVertFile(dir+vertDir+"/cache"+vertFilename+".vsmetal",std::ios::trunc);
    std::ofstream oFragFile(dir+fragDir+"/cache"+fragFilename+".fsmetal",std::ios::trunc);
    if(oVertFile.is_open()){
        oVertFile<<vertSource;
        oVertFile.close();
    }
    if(oFragFile.is_open()){
        oFragFile<<fragSource;
        oFragFile.close();
    }
    //system(("glslangValidator -V --quiet " + out+vertPath + " -o "+out+vertDir+vertFilename+".vert.spv").c_str());
    //system(("glslangValidator -V --quiet " + out+fragPath + " -o "+out+fragDir+fragFilename+".frag.spv").c_str());
    //system(("spirv-cross "+out+vertDir+vertFilename+".vert.spv --msl --output " +out+vertDir+"/cache"+vertFilename + ".vsmetal").c_str());
    //system(("spirv-cross "+out+fragDir+fragFilename+".frag.spv --msl --output " +out+fragDir+"/cache"+fragFilename + ".fsmetal").c_str());
    //system((out+"/bento/shaders/bindfix "+out+vertPath+" "+out+vertDir+"/cache"+vertFilename+".vsmetal ").c_str());
    //system(("rm "+out+vertDir+vertFilename+".vert.spv "+out+fragDir+fragFilename+".frag.spv").c_str());
    //std::cout << " to "+dir+vertDir+"/cache"+vertFilename+".vsmetal and "+dir+fragDir+"/cache"+fragFilename+".fsmetal\n";

    vertShaderSource = [NSString stringWithUTF8String:vertSource.c_str()];
    fragShaderSource = [NSString stringWithUTF8String:fragSource.c_str()];

    id<MTLLibrary> vertLibrary = [device newLibraryWithSource:vertShaderSource options:nil error:&error];
    if(!vertLibrary)std::cerr << "couldn't create vertex shader: " << [[error localizedDescription] UTF8String] << std::endl;

    id<MTLLibrary> fragLibrary = [device newLibraryWithSource:fragShaderSource options:nil error:&error];
    if(!fragLibrary)std::cerr << "couldn't create fragment shader: " << [[error localizedDescription] UTF8String] << std::endl;

    id<MTLFunction> vertexFunction = [vertLibrary newFunctionWithName:@"main0"];
    id<MTLFunction> fragmentFunction = [fragLibrary newFunctionWithName:@"main0"];

    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;

    std::regex colorRegex("\\[\\[color\\((\\d+)\\)\\]\\]");
    std::smatch match;
    int highest = 0;
    std::string::const_iterator start(fragSource.cbegin());
    while (std::regex_search(start, fragSource.cend(), match, colorRegex)) {
        if (match.size()>1)highest = std::max(highest,std::stoi(match[1].str()));
        start = match.suffix().first;
    }
    
    for(int i = 0; i <= highest; i++){
        pipelineDescriptor.colorAttachments[i].pixelFormat = MTLPixelFormatBGRA8Unorm;//MTLPixelFormatRGBA32Float;
        pipelineDescriptor.colorAttachments[i].blendingEnabled = YES;
        pipelineDescriptor.colorAttachments[i].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineDescriptor.colorAttachments[i].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    }

    pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    
    pipelineDescriptor.vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    for(int i = 0; i < vao.attributes.size(); i++){
        switch(vao.attributes[i].format){
            case AttribFormat::Float:
                switch(vao.attributes[i].size){
                    case 1: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatFloat; pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(float);break;
                    case 2: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatFloat2;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(float)*2;break;
                    case 3: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatFloat3;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(float)*3;break;
                    case 4: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatFloat4;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(float)*4;break;
                }
            break;
            case AttribFormat::Int:
                switch(vao.attributes[i].size){
                    case 1: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatInt; pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(int);break;
                    case 2: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatInt2;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(int)*2;break;
                    case 3: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatInt3;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(int)*3;break;
                    case 4: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatInt4;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(int)*4;break;
                }
            break;
            case AttribFormat::UInt:
                switch(vao.attributes[i].size){
                    case 1: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatUInt; pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(uint);break;
                    case 2: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatUInt2;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(uint)*2;break;
                    case 3: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatUInt3;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(uint)*3;break;
                    case 4: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatUInt4;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(uint)*4;break;
                }
            break;
            case AttribFormat::Short:
                switch(vao.attributes[i].size){
                    case 1: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatHalf; pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(short);break;
                    case 2: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatHalf2;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(short)*2;break;
                    case 3: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatHalf3;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(short)*3;break;
                    case 4: pipelineDescriptor.vertexDescriptor.attributes[i].format = MTLVertexFormatHalf4;pipelineDescriptor.vertexDescriptor.layouts[i].stride = sizeof(short)*4;break;
                }
            break;
        }
        pipelineDescriptor.vertexDescriptor.attributes[i].offset = vao.attributes[i].offset;
        pipelineDescriptor.vertexDescriptor.attributes[i].bufferIndex = i;
        pipelineDescriptor.vertexDescriptor.layouts[i].stepRate = 1;
        pipelineDescriptor.vertexDescriptor.layouts[i].stepFunction = MTLVertexStepFunctionPerVertex;
    }
        

    pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    if(!pipelineState)std::cerr << "couldn't create pipeline state: " << [[error localizedDescription] UTF8String] << std::endl;

    int currentOffset = 0;
    int totalSize = 0;
    std::regex inputStructRegex(R"(struct\s+main0_in\s*\{([^}]*)\};)");
    std::regex inputRegex(R"(\s*(\w+\d*)\s+(\w+)\s*\[\[.*?\]\];)");
    std::smatch inputStructMatch;
    if (std::regex_search(vertSource, inputStructMatch, inputStructRegex)) {
        std::string inputStructBody = inputStructMatch[1].str();
        std::sregex_iterator begin(inputStructBody.begin(), inputStructBody.end(), inputRegex);
        std::sregex_iterator end;
        for (auto it = begin; it != end; ++it) {
            std::string type = (*it)[1].str();
            std::string name = (*it)[2].str();
        }
    }
    std::regex structRegex(R"(struct\s+Uniforms\s*\{([^}]*)\};)");
    std::regex uniformRegex(R"(\s*(\w+)\s+(\w+)(\[\d+\])?;)");
    std::smatch structMatch;
    if (std::regex_search(vertSource, structMatch, structRegex)) {
        std::string structBody = structMatch[1].str();
        std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
        std::sregex_iterator end;
        for (auto it = begin; it != end; ++it) {
            std::string type = (*it)[1].str();
            std::string name = (*it)[2].str();
            std::string arraySize = (*it)[3].str();
            int typeSize = 0;
            if(type=="int")typeSize=sizeof(int);
            else if(type=="float")typeSize=sizeof(float);
            else if(type=="float2")typeSize=sizeof(float)*2;
            else if(type=="float3")typeSize=sizeof(float)*4;
            else if(type=="float4")typeSize=sizeof(float)*4;
            else if(type=="packed_float")typeSize=sizeof(float);
            else if(type=="packed_float2")typeSize=sizeof(float)*2;
            else if(type=="packed_float3")typeSize=sizeof(float)*3;
            else if(type=="packed_float4")typeSize=sizeof(float)*4;
            else if(type=="float4x4")typeSize=sizeof(float)*16;
            else if(type=="double")typeSize=sizeof(double);
            else if(type=="double2")typeSize=sizeof(double)*4;
            else if(type=="double3")typeSize=sizeof(double)*4;
            else if(type=="double4")typeSize=sizeof(double)*4;
            int arrayCount = 1;
            if (!arraySize.empty()) {
                std::regex arraySizeRegex(R"(\[(\d+)\])");
                std::smatch arraySizeMatch;
                if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                    arrayCount = std::stoi(arraySizeMatch[1].str());
                }
            }
            uniformMapVert[name] = currentOffset;
            currentOffset += typeSize * arrayCount;
            sizeMapVert[name] = typeSize * arrayCount;
            totalSize += typeSize * arrayCount;
        }
    }
    vertBuffer = [device newBufferWithLength:totalSize options:MTLResourceStorageModeShared];
    currentOffset = 0;
    totalSize = 0;
    if (std::regex_search(fragSource, structMatch, structRegex)) {
        std::string structBody = structMatch[1].str();
        std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
        std::sregex_iterator end;
        for (auto it = begin; it != end; ++it) {
            std::string type = (*it)[1].str();
            std::string name = (*it)[2].str();
            std::string arraySize = (*it)[3].str();
            int typeSize = 0;
            if(type=="int")typeSize=sizeof(int);
            else if(type=="float")typeSize=sizeof(float);
            else if(type=="float2")typeSize=sizeof(float)*2;
            else if(type=="float3")typeSize=sizeof(float)*4;
            else if(type=="float4")typeSize=sizeof(float)*4;
            else if(type=="packed_float")typeSize=sizeof(float);
            else if(type=="packed_float2")typeSize=sizeof(float)*2;
            else if(type=="packed_float3")typeSize=sizeof(float)*3;
            else if(type=="packed_float4")typeSize=sizeof(float)*4;
            else if(type=="float4x4")typeSize=sizeof(float)*16;
            else if(type=="double")typeSize=sizeof(double);
            else if(type=="double2")typeSize=sizeof(double)*4;
            else if(type=="double3")typeSize=sizeof(double)*4;
            else if(type=="double4")typeSize=sizeof(double)*4;
            int arrayCount = 1;
            if (!arraySize.empty()) {
                std::regex arraySizeRegex(R"(\[(\d+)\])");
                std::smatch arraySizeMatch;
                if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                    arrayCount = std::stoi(arraySizeMatch[1].str());
                }
            }
            uniformMapFrag[name] = currentOffset;
            currentOffset += typeSize * arrayCount;
            sizeMapFrag[name] = typeSize * arrayCount;
            totalSize += typeSize * arrayCount;
        }
    }
    fragBuffer = [device newBufferWithLength:totalSize options:MTLResourceStorageModeShared];
}

std::string Shader::convertShader(std::string source,EShLanguage lang,std::string name){
    glslang::InitializeProcess();
    glslang::TShader shader(lang);
    
    const char* src = source.c_str();
    shader.setStrings(&src,1);
    shader.setEnvInput(glslang::EShSourceGlsl,lang,glslang::EShClientOpenGL,410);
    shader.setEnvClient(glslang::EShClientVulkan,glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv,glslang::EShTargetSpv_1_5);

    if(!shader.parse(GetDefaultResources(),410,false,EShMsgDefault))std::cerr << "couldn't convert shaders\n"<<name<< shader.getInfoLog() << std::endl;

    glslang::TProgram program;
    program.addShader(&shader);
    program.link(EShMsgDefault);

    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(lang), spirv);
    glslang::FinalizeProcess();

    spirv_cross::CompilerMSL compiler(spirv);

    for(spirv_cross::Resource uniformBuffer:compiler.get_shader_resources().uniform_buffers){//cool
        uint32_t set = compiler.get_decoration(uniformBuffer.id, spv::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);

        spirv_cross::MSLResourceBinding resourceBinding;
        resourceBinding.stage = lang==EShLangVertex?spv::ExecutionModelVertex:spv::ExecutionModelFragment;
        resourceBinding.desc_set = 0;
        resourceBinding.binding = set*16+binding;
        resourceBinding.msl_buffer = set*16+binding;

        compiler.add_msl_resource_binding(resourceBinding);
    }

    std::string file = compiler.compile();

    return file;
}