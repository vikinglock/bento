#include "shader.h"

std::string Shader::getUni(){
    std::string out;
    auto uniforms = extractUniforms(program);
    for (const auto& [name, index] : uniforms) {
        out.append(name);
        out.append(" : ");
        out.append(std::to_string(index));
        out.append("\n");
    }
    return out;
}

__attribute__((weak))
void Shader::createShader(){
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

__attribute__((weak))
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

    vertSource = loadFileString(dir+vertDir+"/cache"+vertFilename+".vs");
    fragSource = loadFileString(dir+fragDir+"/cache"+fragFilename+".fs");

    if(loaded)createShader();
}

/*VAO::VAO(){
    glGenVertexArrays(1, &index);
    vaos.insert(index);
}

void VAO::setAttrib(int ind, int size, AttribFormat attribFormat, int offset){
    if(ind >= buffers.size()){buffers.resize(ind+1);sizes.resize(ind+1);}

    glGenBuffers(1, &buffers[ind]);
    buffs.insert(buffers[ind]);
    sizes[ind] = size;

    glBindVertexArray(index);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[ind]);
    glVertexAttribPointer(ind,size,(unsigned int)attribFormat,GL_FALSE,0,(void*)(intptr_t)offset);
    glEnableVertexAttribArray(ind);

}*/

GLuint Shader::compileShader(GLenum type,std::string source){
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
    if(!success){//mostly useless now that spirv-cross'll tell me any errors i make
        GLint logLength;
        glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader,logLength,nullptr,log.data());
        std::cerr<<"couldn't create shaders\nerror log:\n"<<log.data()<<std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}
GLuint Shader::createShaderProgram(std::string vertSource,std::string fragSource){
    GLuint vertexShader = Shader::compileShader(GL_VERTEX_SHADER,vertSource);
    GLuint fragmentShader = Shader::compileShader(GL_FRAGMENT_SHADER,fragSource);

    GLuint program = glCreateProgram();

    glAttachShader(program,vertexShader);//reminds me of my endeavors in webgl
    glAttachShader(program,fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success){
        GLint logLength;
        glGetProgramiv(program,GL_INFO_LOG_LENGTH,&logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(program,logLength,nullptr,log.data());
        std::cerr<<"couldn't link program\nerror log:\n"<<log.data()<<std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

std::unordered_map<std::string,int> Shader::extractUniforms(GLuint program){
    std::unordered_map<std::string,int> uniformMap;
    GLint numUniforms;
    glGetProgramiv(program,GL_ACTIVE_UNIFORMS,&numUniforms);
    
    for(GLint i = 0; i < numUniforms; i++){
        char name[128];
        GLint size;
        GLenum type;
        glGetActiveUniform(program,i,sizeof(name),nullptr,&size,&type,name);
        int location = glGetUniformLocation(program,name);
        if(location!=-1)
            uniformMap[name] = location;
    }
    
    return uniformMap;
}

Shader::~Shader(){
    glDeleteProgram(program);
}