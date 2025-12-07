#include "shader.h"


void Shader::createShader(){
    vertSource = Shader::convertShader(vertSource,EShLangVertex,queue.vname);
    fragSource = Shader::convertShader(fragSource,EShLangFragment,queue.fname);

    std::ofstream oVertFile(queue.vpath,std::ios::trunc);
    std::ofstream oFragFile(queue.fpath,std::ios::trunc);
    if(oVertFile.is_open()){
        oVertFile<<vertSource;
        oVertFile.close();
    }
    if(oFragFile.is_open()){
        oFragFile<<fragSource;
        oFragFile.close();
    }
    
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

    program = createShaderProgram(vertSource,fragSource);


    std::smatch match;
    auto begin = fragSource.cbegin();
    auto end = fragSource.cend();
    std::regex textureFindThingRegex(R"((sampler\w+)\s+(\w+);)");
    int index = 0;
    while (std::regex_search(begin, end, match,textureFindThingRegex)) {
        std::string type = match[1].str();
        std::string name = match[2].str();
        textureLocs[index] = glGetUniformLocation(program, name.c_str());
        index++;
        begin = match.suffix().first;
    }
}

Shader::Shader(std::string vertPath, std::string fragPath, VAO vaot){
    vao = vaot;

    vertPath = "/"+vertPath;
    size_t lastSlash = vertPath.find_last_of("/\\");
    std::string vertDir = "";
    std::string vertFilename = "";
    if (lastSlash != std::string::npos) {
        vertDir = vertPath.substr(0, lastSlash);
        vertFilename = vertPath.substr(lastSlash, vertPath.rfind('.') - lastSlash);
    }
    fragPath = "/"+fragPath;
    lastSlash = fragPath.find_last_of("/\\");
    std::string fragDir = "";
    std::string fragFilename = "";
    if (lastSlash != std::string::npos) {
        fragDir = fragPath.substr(0, lastSlash);
        fragFilename = fragPath.substr(lastSlash, fragPath.rfind('.') - lastSlash);
    }

    std::string dir = getExecutablePath();
    vertSource = loadFileString(dir+vertPath);
    fragSource = loadFileString(dir+fragPath);
    
    queue.vname = vertFilename;
    queue.fname = fragFilename;
    queue.vpath = dir+vertDir+"/cache"+vertFilename+".vs";
    queue.fpath = dir+fragDir+"/cache"+fragFilename+".fs";

    //system(("glslangValidator -V --quiet " + dir+vertPath + " -o "+dir+vertDir+vertFilename+".vert.spv").c_str());
    //system(("glslangValidator -V --quiet " + dir+fragPath + " -o "+dir+fragDir+fragFilename+".frag.spv").c_str());
    //system(("spirv-cross "+dir+vertDir+vertFilename+".vert.spv --version 330 --output " +dir+vertDir+"/cache"+vertFilename + ".vs").c_str());
    //system(("spirv-cross "+dir+fragDir+fragFilename+".frag.spv --version 330 --output " +dir+fragDir+"/cache"+fragFilename + ".fs").c_str());

    //fix330cErrors(dir+vertDir+"/cache"+vertFilename+".vs");
    //fix330cErrors(dir+fragDir+"/cache"+fragFilename+".fs");
    //std::remove((dir+vertDir+vertFilename+".vert.spv ").c_str());
    //std::remove((dir+fragDir+fragFilename+".frag.spv").c_str());

    //std::cout << " to "+dir+vertDir+"/cache"+vertFilename+".vs and "+dir+fragDir+"/cache"+fragFilename+".fs\n";

    if(loaded)createShader();
}

std::string Shader::convertShader(std::string source,EShLanguage lang,std::string name){
    glslang::InitializeProcess();// :)
    glslang::TShader shader(lang);
    
    const char* src = source.c_str();
    shader.setStrings(&src,1);
    shader.setEnvInput(glslang::EShSourceGlsl,lang,glslang::EShClientOpenGL,410);
    shader.setEnvClient(glslang::EShClientVulkan,glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv,glslang::EShTargetSpv_1_5);

    if(!shader.parse(GetDefaultResources(),410,false,EShMsgDefault))std::cerr<<"couldn't convert shaders\n"<<name<<" error log: "<<shader.getInfoLog()<<std::endl;

    glslang::TProgram prog;
    prog.addShader(&shader);
    prog.link(EShMsgDefault);

    std::vector<uint32_t> spirv;//i do appreciate how c++ lets you do this
    glslang::GlslangToSpv(*prog.getIntermediate(lang),spirv);
    glslang::FinalizeProcess();

    spirv_cross::CompilerGLSL compiler(spirv);

    spirv_cross::CompilerGLSL::Options options;
    options.version = 330;
    options.es = false;
    options.vertex.fixup_clipspace = true;
    options.vertex.flip_vert_y = true;
    compiler.set_common_options(options);

    std::string file = compiler.compile();



    //just ignore all this eh
    
    file = std::regex_replace(file, std::regex("#version 330"),"#version 330 core");
    std::string uniforms;
    std::regex uniformBlockPattern("layout\\s*\\([^)]*\\)\\s*uniform\\s+(\\w+)\\s*\\{([^}]+)\\}\\s*(\\w+)\\s*;");
    std::regex uniformPattern("\\s*(\\w+)\\s+(\\w+(?:\\s*\\[\\d+\\])?)\\s*;");
    std::regex arrayPattern("(\\w+)\\s*\\[(\\d+)\\]");
    std::smatch match;
    std::smatch arrayMatch;
    while(std::regex_search(file,match,uniformBlockPattern)){
        std::string uniformBlockContent = match[2].str();
        std::string::const_iterator uniformBlockSearch(uniformBlockContent.cbegin());
        std::smatch uniformMatch;
        while(std::regex_search(uniformBlockSearch,uniformBlockContent.cend(),uniformMatch,uniformPattern)){
            std::string type = uniformMatch[1].str();
            std::string name = uniformMatch[2].str();
            type.erase(std::remove(type.begin(),type.end(),'*'),type.end());
            name.erase(std::remove(name.begin(),name.end(),'*'),name.end());

            if(std::regex_search(name,arrayMatch,arrayPattern)) uniforms += "uniform "+type+" "+arrayMatch[1].str()+"["+arrayMatch[2].str()+"];\n";
            else uniforms += "uniform " + type + " " + name + ";\n";
            uniformBlockSearch = uniformMatch.suffix().first;
        }

        file = match.prefix().str()+match.suffix().str();
    }
    std::regex layoutUniformPattern("layout\\s*\\([^)]*\\)\\s*uniform\\s+(\\w+)\\s+(\\w+(?:\\s*\\[\\d+\\])?)\\s*;");
    while(std::regex_search(file,match,layoutUniformPattern)){//get rid of layout(binding = __) uniform sampler2D __;
        std::string type = match[1].str();
        std::string name = match[2].str();
        type.erase(std::remove(type.begin(),type.end(),'*'),type.end());
        name.erase(std::remove(name.begin(),name.end(),'*'),name.end());

        if(std::regex_search(name,arrayMatch,arrayPattern))
            uniforms += "uniform "+type+" "+arrayMatch[1].str()+"["+arrayMatch[2].str()+"];\n";
        else
            uniforms += "uniform "+type+" "+name+";\n";

        file = match.prefix().str()+match.suffix().str();
    }
    
    std::regex strrref("(_\\d+)\\.");
    file = std::regex_replace(file, strrref, "");

    size_t first = file.find_first_not_of(" \t\n\r");
    size_t last = file.find_last_not_of(" \t\n\r");
    file = file.substr(first, last - first + 1);

    if(uniforms.length()>0){
        size_t first = uniforms.find_first_not_of(" \t\n\r");
        size_t last = uniforms.find_last_not_of(" \t\n\r");
        file.insert(file.find("#endif\n")+7,uniforms.substr(first,last-first+1)+"\n");
    }

    std::regex newlines("\n\\s*\n");
    file = std::regex_replace(file,newlines,"\n");

    //donezo
    return file;
}