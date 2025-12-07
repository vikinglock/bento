#include "opengl.h"
#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/gtc/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"
#include "../../utils.h"


#include <limits.h>
#include <string>

#ifdef MACOS
#include <unistd.h>
#include <mach-o/dyld.h>
#elif WINDOWS
#include <windows.h>
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "../../lib/miniaudio/miniaudio.h"

void loop();
void exit();

std::vector<GLuint> buffers;


bool useDefShader = true;

std::vector<bool> normalizedTextures;

enum {
    KeyStateNone,
    KeyStatePressed,
    KeyStateReleased
};

bool shouldClose;
int vertCount = 0;
int normCount = 0;
int uvCount = 0;
int wheelX = 0;
int wheelY = 0;

std::string vertShaderSource="",fragShaderSource="";

glm::vec4 clearColor = glm::vec4(0.0,0.0,0.0,1.0);

int buttonCount;
const unsigned char* buttons[GLFW_JOYSTICK_LAST];
int axisCount;
const float* axes[GLFW_JOYSTICK_LAST];
//opengl is so much more straightforward
//metal has grown on me though

bool loaded = false;



Texture* Bento::AppTexture;
FrameBuffer* Bento::DefaultFrameBuffer;
Shader* Bento::DefaultShader;

/*#ifdef MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#include "../../lib/GLFW/glfw3native.h"
#include <Cocoa/Cocoa.h>
#endif*/

// #### MAIN ####
Bento::Bento(const char *title,int width,int height,int x,int y){
    #ifdef FREEZE_FILES
    File::loadFrozenFilesystem("resources.fz");
    #endif

    if(!glfwInit()){
        const char* error;//can someone tell me if this fails cuz i don't know how to check
        glfwGetError(&error);
        std::cerr << "couldn't initialize glfw: " << error << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    #ifdef MACOS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    #endif
    #ifdef WINDOWS
    glfwWindowHint(GLFW_RAW_MOUSE_MOTION,GLFW_TRUE);
    #endif

    window = glfwCreateWindow(width,height,title,nullptr,nullptr);

    
    /*#ifdef MACOS
    NSWindow* nswin = glfwGetCocoaWindow(window);
    nswin.styleMask = NSWindowStyleMaskBorderless;
    nswin.contentView.layer.cornerRadius = 0.0;
    #endif*/

    setWindowPos(glm::vec2(x,y));
    
    if(!window){
        const char* error;
        glfwGetError(&error);
        std::cerr << "couldn't create window: " << error << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        const char* error;
        glfwGetError(&error);
        std::cerr << "couldn't initialize glad (not glad for this one):" << error << std::endl;
        return;
    }
    loaded = true;

    VAO vao;
    vao.setAttrib(0,2,AttribFormat::Float);
    vao.setAttrib(1,2,AttribFormat::Float);

    
    vertShaderSource = "#version 330 core\n\
    out vec2 fuv;\
    layout(location=0)in vec2 pos;\
    layout(location=1)in vec2 uv;\
    void main(){\
        gl_Position = vec4(pos,0.0,1.0);\
        fuv = uv;\
    }";
    fragShaderSource = "#version 330 core\n\
    uniform sampler2D tex;\
    in vec2 fuv;\
    out vec4 col;\
    void main(){\
        col=texture(tex,fuv);\
    }";
    GLuint shd = Shader::createShaderProgram(vertShaderSource,fragShaderSource);
    Bento::DefaultShader = new Shader(shd,vertShaderSource,fragShaderSource,vao);

    std::smatch match;
    auto begin = fragShaderSource.cbegin();
    auto end = fragShaderSource.cend();
    std::regex textureFindThingRegex(R"((sampler\w+)\s+(\w+);)");
    int index = 0;
    while(std::regex_search(begin,end,match,textureFindThingRegex)){
        std::string type = match[1].str();
        std::string name = match[2].str();
        Bento::DefaultShader->textureLocs[index] = glGetUniformLocation(Bento::DefaultShader->getProgram(),name.c_str());
        index++;
        begin = match.suffix().first;
    }

    setShader(Bento::DefaultShader);

    dTInd = 0;
    depthTexture.resize(1);
    glGenTextures(1,&depthTexture[0]);

    for(int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i){
        if(glfwJoystickPresent(i)){
            buttons[i] = glfwGetJoystickButtons(i,&buttonCount);
            axes[i] = glfwGetJoystickAxes(i,&axisCount);
        }
    }

    glGenBuffers(1,&vertexBuffer);
    glGenBuffers(1,&normalBuffer);
    glGenBuffers(1,&uvBuffer);
    glGenTextures(1,&depthRTexture);


    Bento::AppTexture = new Texture(nullptr,glm::ivec2(width,height));

    Bento::DefaultFrameBuffer = new FrameBuffer(Bento::AppTexture);
    Bento::DefaultFrameBuffer->size = glm::ivec2(width,height);
    bindFrameBuffer(Bento::DefaultFrameBuffer);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glfwSetScrollCallback(window,[](GLFWwindow *window,double xoffset,double yoffset){
        wheelX = xoffset;
        wheelY = yoffset;
        #ifdef IMGUI
        //ImGui_ImplGlfw_ScrollCallback(window,xoffset,yoffset);
        #endif
    });
    glfwSetFramebufferSizeCallback(window,[](GLFWwindow *window,int width,int height){
        Bento::DefaultFrameBuffer->resize(glm::ivec2(width,height));
        if(resizeCallback)resizeCallback(glm::ivec2(width,height));
    });
    glfwSetWindowRefreshCallback(window,[](GLFWwindow *window){
        //glfwSwapBuffers(window);
    });
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
}

ma_engine engine;

void Bento::initSound(){
    ma_engine_init(NULL,&engine);
}


void Bento::setClearColor(glm::vec4 col){
    clearColor = col;
}

void Bento::focus(){
    glfwFocusWindow(window);
}

void Bento::setVertices(const std::vector<glm::vec3>& vs) {
    vertices = vs;
    glBindVertexArray(shader->vaoIndex);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,vertices.size() * sizeof(glm::vec3),vertices.data(),GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,nullptr);
    glEnableVertexAttribArray(0);
}

void Bento::setNormals(const std::vector<glm::vec3>& ns) {
    normals = ns;
    glBindVertexArray(shader->vaoIndex);
    glBindBuffer(GL_ARRAY_BUFFER,normalBuffer);
    glBufferData(GL_ARRAY_BUFFER,normals.size() * sizeof(glm::vec3),normals.data(),GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,nullptr);
    glEnableVertexAttribArray(1);
}

void Bento::setUvs(const std::vector<glm::vec2>& uv){
    uvs = uv;
    glBindVertexArray(shader->vaoIndex);
    glBindBuffer(GL_ARRAY_BUFFER,uvBuffer);
    glBufferData(GL_ARRAY_BUFFER,uvs.size() * sizeof(glm::vec2),uvs.data(),GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,nullptr);
    glEnableVertexAttribArray(2);
}

void Bento::internalSetVertexBuffer(int index,void* data,size_t size,size_t typeSize){
    glBindVertexArray(shader->vaoIndex);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[index]);
    glBufferData(GL_ARRAY_BUFFER,size*typeSize,data,GL_DYNAMIC_DRAW);
    if(index==0)vertexAmount = size;
}

void Bento::bindTexture(Texture *texture,int ind){
    ///for(auto [a,b] : shader->textureLocs)std::cout<<a<<":"<<b<<std::endl;
    glUniform1i(shader->textureLocs[ind],ind);
    glActiveTexture(GL_TEXTURE0+ind);
    glBindTexture((GLuint)texture->texType,texture->getTexture());
}

void Bento::unbindTexture(){
    glBindTexture(GL_TEXTURE_2D,0);
}

void Bento::bindFrameBuffer(FrameBuffer* framebuffer){
    this->framebuffer = framebuffer;
}
void Bento::poll(){
    glm::vec2 pos = getMousePosition();
    if(mouseLocked){
        mouseDelta = pos-lockMousePos;
    }else{
        static glm::vec2 lastPos(pos);
        mouseDelta = pos-lastPos;
        lastPos = pos;
    }

    if(mouseLocked)setMousePosition(lockMousePos);

    for(int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i)prevKeyStates[i] = getKey(i);
    for(int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; ++i)prevMouseStates[i] = getMouse(i);
    glfwPollEvents();
}

void Bento::predraw(){
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer->getFBO());

    glClearColor(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    if(clearColor.w!=0)glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i){
        if (glfwJoystickPresent(i)) {
            buttons[i] = glfwGetJoystickButtons(i,&buttonCount);
            axes[i] = glfwGetJoystickAxes(i,&axisCount);
        }
    }
    glUseProgram(shader->getProgram());

    if(clearColor.w!=0)
        for(int i = 0; i < framebuffer->attachments.size(); i++) {
            glClearBufferfv(GL_COLOR,i,glm::value_ptr(clearColor));
        }
    float clearDepth = 1.0f;
    glClearBufferfv(GL_DEPTH,0,&clearDepth);
    
    if(framebuffer->size.x>0&&framebuffer->size.y>0)glViewport(0,0,framebuffer->size.x,framebuffer->size.y);
    else glViewport(0,0,Bento::DefaultFrameBuffer->size.x,Bento::DefaultFrameBuffer->size.y);
}
void Bento::setShader(Shader* shd){
    useDefShader = shd==Bento::DefaultShader;
    shader = shd;
}

void Bento::draw(Primitive primitive){
    if(this->framebuffer->depthEnabled)glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    glBindVertexArray(shader->vaoIndex);
    glDrawBuffers(framebuffer->attachments.size(),framebuffer->attachments.data());
    if(vertexAmount>0)glDrawArrays((GLuint)primitive,0,vertexAmount);
}

void Bento::render(){    
    glfwSwapBuffers(window);
}

void Bento::drawScreen(Shader* shd){//cuz i'm lazy
    glUseProgram(shd->getProgram());
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    bindTexture(Bento::AppTexture,0);
    glViewport(0,0,Bento::DefaultFrameBuffer->size.x,Bento::DefaultFrameBuffer->size.y);

    std::vector<glm::vec2> buf = {
        glm::vec2(1,-1),glm::vec2(-1,-1),glm::vec2(-1,1),
        glm::vec2(1,-1),glm::vec2(-1,1),glm::vec2(1,1)
    };
    std::vector<glm::vec2> uvbuf = {
        glm::vec2(1,0),glm::vec2(0,0),glm::vec2(0,1),
        glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1)
    };
    setVertexBuffer(0,buf);
    setVertexBuffer(1,uvbuf);
    glBindVertexArray(shd->vaoIndex);
    glDrawArrays(GL_TRIANGLES,0,6);
}

void Bento::setActiveTextures(int start,int end){
    startRT = start;
    endRT = end+1;
    for(int i = texture.size(); i < end; i++){
        normalizedTextures.push_back(true);
    }
}
void Bento::setActiveTextures(int ind){
    startRT = ind;
    endRT = ind+1;
    for(int i = texture.size(); i < ind; i++){
        normalizedTextures.push_back(true);
    }
}

void Bento::setActiveDepthTexture(int ind){
    if(depthTexture.size()<ind){
        depthTexture.resize(ind+1);
    }
    dTInd = ind;
}

void Bento::setActiveAttachments(int start,int end){
    startAtt = start;
    endAtt = end+1;
}
void Bento::setActiveAttachments(int ind){
    startAtt = ind;
    endAtt = ind+2;
}

void Bento::predrawTex(int width,int height) {
    glCullFace(GL_FRONT);
    //glGenFramebuffers(1,&framebuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
    if(!depthTexture[dTInd]){
        glGenTextures(1,&depthTexture[dTInd]);
    }
    glBindTexture(GL_TEXTURE_2D,depthTexture[dTInd]);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,depthTexture[dTInd],0);
    for(int i = 0; i < (endRT-startRT); i++){
        if (startRT+i >= texture.size()) {
            texture.resize(startRT+i + 1);
            glGenTextures(1,&texture[startRT+i]);
        }
        glBindTexture(GL_TEXTURE_2D,texture[startRT+i]);
        if(normalizedTextures[i]){
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
        }else{
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,width,height,0,GL_RGB,GL_FLOAT,NULL);
        }
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+startAtt+1+i,GL_TEXTURE_2D,texture[startRT+i],0);
    }

    glBindTexture(GL_TEXTURE_2D,depthRTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32F,width,height,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depthRTexture,0);
    glGetIntegerv(GL_VIEWPORT,tvp);
    glViewport(0,0,width,height);
    

    static std::vector<GLuint> attachments;
    attachments.resize((endAtt-startAtt) + 1);
    attachments[0] = GL_COLOR_ATTACHMENT0;
    for(int i = 1; i < (endAtt-startAtt) + 1; i++){
        attachments[i] = GL_COLOR_ATTACHMENT0+i+startAtt;
    }
    glDrawBuffers(endAtt-startAtt,attachments.data());

    float clearColorDepth[4] = {0.0,0.0,0.0,1.0};
    glClearBufferfv(GL_COLOR,0,clearColorDepth);
    
    float clearColorValues[4] = {clearColor.x,clearColor.y,clearColor.z,clearColor.w};
    for(int i = startAtt+1; i < endAtt; i++) {
        glClearBufferfv(GL_COLOR,i,clearColorValues);
    }
    float clearDepth = 1.0f;
    glClearBufferfv(GL_DEPTH,0,&clearDepth);
}

void Bento::drawTex() {
    static std::vector<GLuint> attachments;
    attachments.resize((endAtt-startAtt) + 1);
    attachments[0] = GL_COLOR_ATTACHMENT0;
    for(int i = 1; i < (endAtt-startAtt) + 1; i++){
        attachments[i] = GL_COLOR_ATTACHMENT0+i+startAtt;
    }
    glDrawBuffers(endAtt-startAtt,attachments.data());

    glBindVertexArray(shader->vaoIndex);
    glDrawArrays(GL_TRIANGLES,0,vertexAmount);
}

void Bento::renderTex(){
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(tvp[0],tvp[1],tvp[2],tvp[3]);
    glCullFace(GL_FRONT);
}

void Bento::renderToTex(Texture*& tex,int ind){
    tex = new Texture(texture[ind]);
}

void Bento::renderDepthToTex(Texture*& tex,int ind){
    tex = new Texture(depthTexture[ind]);
}

void Bento::renderToTex(Texture*& tex1,Texture*& tex2,int ind){
    tex1 = new Texture(texture[ind]);
    tex2 = new Texture(texture[ind+1]);
}

void Bento::renderToTex(Texture*& tex1,Texture*& tex2,Texture*& tex3,int ind){
    tex1 = new Texture(texture[ind]);
    tex2 = new Texture(texture[ind+1]);
    tex3 = new Texture(texture[ind+2]);
}
void Bento::normalizeTexture(int index,bool normalized){
    normalizedTextures[index] = normalized;
}
bool Bento::isRunning(){return !glfwWindowShouldClose(window);}

// #### INPUT ####

__attribute__((weak))
void Bento::exit() {
    ma_engine_uninit(&engine);


    glfwTerminate();
    for(GLuint vao : vaos)glDeleteVertexArrays(1,&vao);
    for(GLuint buffer : buffs)glDeleteBuffers(1,&buffer);
    std::exit(0);
}

void Bento::toggleFullscreen() {
    
}
bool Bento::isWindowFocused() {
    return glfwGetWindowAttrib(window,GLFW_FOCUSED) != 0;
}
// #### INPUT ####
bool Bento::getKey(int key) {
    return glfwGetKey(window,key) == GLFW_PRESS;
}
bool Bento::getMouse(int mouse) {
    return glfwGetMouseButton(window,mouse) == GLFW_PRESS;
}

bool Bento::getKeyDown(int key) {
    bool curr = getKey(key);
    bool prev = prevKeyStates[key];
    return curr && !prev;
}
bool Bento::getKeyUp(int key) {
    bool curr = getKey(key);
    bool prev = prevKeyStates[key];
    return !curr && prev;
}
bool Bento::getMouseDown(int button) {
    bool curr = getMouse(button);
    bool prev = prevMouseStates[button];
    return curr && !prev;
}
bool Bento::getMouseUp(int button) {
    bool curr = getMouse(button);
    bool prev = prevMouseStates[button];
    return !curr && prev;
}


double Bento::getScroll(int wheel){
    if(wheel == 0){
        float temp = wheelY;
        wheelY = 0;
        return temp;
    }else if(wheel == 1){
        float temp = wheelX;
        wheelX = 0;
        return temp;
    }
    return 0;
}

// #### MOUSE AND WINDOWS ####
glm::vec2 Bento::getWindowSize() {
    int width,height;
    glfwGetWindowSize(window,&width,&height);
    return glm::vec2(width,height);
}
glm::vec2 Bento::getFramebufferSize() {
    int width,height;
    glfwGetFramebufferSize(window,&width,&height);
    return glm::vec2(width,height);
}


glm::vec2 Bento::getWindowPos() {
    int x,y;
    glfwGetWindowPos(window,&x,&y);
    return glm::vec2(x,y);
}

void Bento::setMouseCursor(bool hide,int cursor) {
    if(hide)glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
    else    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
}
void Bento::lockMouse(bool locked){
    mouseLocked = locked;
    lockMousePos = glm::ivec2(getMousePosition());
}

glm::vec2 Bento::getMousePosition() {
    double x,y;
    glfwGetCursorPos(window,&x,&y);
    return glm::vec2(x,y)+getWindowPos();
}
glm::vec2 Bento::getMouseDelta(){return mouseDelta;}

void Bento::setMousePosition(glm::vec2 pos,bool needsFocus) {
    if(!needsFocus||!isWindowFocused()){
        glm::vec2 windowPos = getWindowPos();
        glfwSetCursorPos(window,pos.x-windowPos.x,pos.y-windowPos.y);
    }
}


glm::vec2 Bento::getControllerAxis(int controller,JoystickType joystick) {
    if(axes[controller]==nullptr||axisCount<4){
        std::cerr << "error with the controller axes (like the joysticks and stuff)" << std::endl;
        return glm::vec2(0);
    }
    switch(joystick){
        case GAMEPAD_JOYSTICK_LEFT: return glm::vec2(axes[controller][0],axes[controller][1]);
        case GAMEPAD_JOYSTICK_RIGHT:return glm::vec2(axes[controller][2],axes[controller][3]);
        default:                    return glm::vec2(0);
    }
}
bool Bento::getControllerButton(int controller,ButtonType button){
    return buttons[controller][button]==GLFW_PRESS;
}

void Bento::setWindowPos(glm::vec2 pos){
    #ifdef WIN32
    pos.y+=GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CXPADDEDBORDER);
    #endif
    glfwSetWindowPos(window,pos.x,pos.y);
}

glm::vec2 Bento::getDisplaySize() {
    int width,height;
    glfwGetFramebufferSize(window,&width,&height);
    return glm::vec2(width,height);
}

void (*Bento::keyCallback)(char,KeyCode) = nullptr;
void (*Bento::resizeCallback)(glm::ivec2) = nullptr;

void Bento::setKeyCallback(void (*callback)(char,KeyCode)){keyCallback = callback;}
void Bento::setResizeCallback(void (*callback)(glm::ivec2)){resizeCallback = callback;}

    
void Bento::addLight(const glm::vec3 pos,const glm::vec3 ambient,const glm::vec3 diffuse,const glm::vec3 specular,float constant,float linear,float quadratic) {
    if (numLights >= MAX_LIGHTS) return;

    positions[numLights] = pos;
    constants[numLights] = constant;
    linears[numLights] = linear;
    quads[numLights] = quadratic;
    ambients[numLights] = ambient;
    diffuses[numLights] = diffuse;
    speculars[numLights] = specular;

    numLights++;
}

void Bento::setLightPos(int index,glm::vec3 position){positions[index] = position;}
void Bento::setLightConstants(int index,float constant){constants[index] = constant;}
void Bento::setLightLinears(int index,float linear){linears[index] = linear;}
void Bento::setLightQuads(int index,float quad){quads[index] = quad;}
void Bento::setLightAmbients(int index,glm::vec3 ambient){ambients[index] = ambient;}
void Bento::setLightDiffuses(int index,glm::vec3 diffuse){diffuses[index] = diffuse;}
void Bento::setLightSpeculars(int index,glm::vec3 specular){speculars[index] = specular;}


void Bento::setAmbientColor(glm::vec3 ambient){amb = ambient;}




std::string Bento::getFramework(){
    return "OpenGL";
}

void Bento::enable(Feature f,bool enabled){
    switch(f){
        case 0:glfwSwapInterval(0);break;
        case 1:if(enabled)glDepthMask(GL_FALSE);else glDepthMask(GL_TRUE);break;
        case 2:case 3://idk just do some push ups or something
        break;
    }
}
std::mutex glLock;
void Bento::startLoop(){
    while(isRunning())loop();
    exit();
}

glm::vec2 Bento::getTouchPos(int index){
    return glm::vec2(0);
}
bool Bento::getTouch(int index){
    return false;
}
bool Bento::getTouchDown(int index){
    return false;
}
bool Bento::getTouchUp(int index){
    return false;
}

std::string Bento::getOperatingSystem(){
    #ifdef WINDOWS
        return "Windows";
    #elif MACOS
        return "Macos";
    #elif LINUX
        return "Linux";
    #endif
    return "";
}

void Bento::setUniform(std::string uniformName,float value,bool onVertex){glUniform1f(glGetUniformLocation(shader->program,uniformName.c_str()),value);}
void Bento::setUniform(std::string uniformName,int value,bool onVertex){glUniform1i(glGetUniformLocation(shader->program,uniformName.c_str()),value);}
void Bento::setUniform(std::string uniformName,glm::vec2 value,bool onVertex){glUniform2fv(glGetUniformLocation(shader->program,uniformName.c_str()),1,&value[0]);}
void Bento::setUniform(std::string uniformName,glm::vec3 value,bool onVertex){glUniform3fv(glGetUniformLocation(shader->program,uniformName.c_str()),1,&value[0]);}
void Bento::setUniform(std::string uniformName,glm::mat4 value,bool onVertex){glUniformMatrix4fv(glGetUniformLocation(shader->program,uniformName.c_str()),1,GL_FALSE,&value[0][0]);}
void Bento::setUniform(std::string uniformName,glm::vec4 value,bool onVertex){glUniform4fv(glGetUniformLocation(shader->program,uniformName.c_str()),1,&value[0]);}

void Bento::setUniform(std::string uniformName,float* values,int count,bool onVertex){glUniform1fv(glGetUniformLocation(shader->program,uniformName.c_str()),count,values);}
void Bento::setUniform(std::string uniformName,int* values,int count,bool onVertex){glUniform1iv(glGetUniformLocation(shader->program,uniformName.c_str()),count,values);}
void Bento::setUniform(std::string uniformName,glm::vec2* values,int count,bool onVertex){glUniform2fv(glGetUniformLocation(shader->program,uniformName.c_str()),count,glm::value_ptr(values[0]));}
void Bento::setUniform(std::string uniformName,glm::vec3* values,int count,bool onVertex){glUniform3fv(glGetUniformLocation(shader->program,uniformName.c_str()),count,glm::value_ptr(values[0]));}
void Bento::setUniform(std::string uniformName,glm::vec4* values,int count,bool onVertex){glUniform4fv(glGetUniformLocation(shader->program,uniformName.c_str()),count,glm::value_ptr(values[0]));}
void Bento::setUniform(std::string uniformName,glm::mat4* values,int count,bool onVertex){glUniformMatrix4fv(glGetUniformLocation(shader->program,uniformName.c_str()),count,GL_FALSE,glm::value_ptr(values[0][0]));}

void Bento::setUniform(std::string uniformName,const std::vector<float>& values,bool onVertex){glUniform1fv(glGetUniformLocation(shader->program,uniformName.c_str()),values.size(),values.data());}
void Bento::setUniform(std::string uniformName,const std::vector<int>& values,bool onVertex){glUniform1iv(glGetUniformLocation(shader->program,uniformName.c_str()),values.size(),values.data());}
void Bento::setUniform(std::string uniformName,const std::vector<glm::vec2>& values,bool onVertex){glUniform2fv(glGetUniformLocation(shader->program,uniformName.c_str()),values.size(),glm::value_ptr(values[0]));}
void Bento::setUniform(std::string uniformName,const std::vector<glm::vec3>& values,bool onVertex){glUniform3fv(glGetUniformLocation(shader->program,uniformName.c_str()),values.size(),glm::value_ptr(values[0]));}
void Bento::setUniform(std::string uniformName,const std::vector<glm::vec4>& values,bool onVertex){glUniform4fv(glGetUniformLocation(shader->program,uniformName.c_str()),values.size(),glm::value_ptr(values[0]));}
void Bento::setUniform(std::string uniformName,const std::vector<glm::mat4>& values,bool onVertex){glUniformMatrix4fv(glGetUniformLocation(shader->program,uniformName.c_str()),values.size(),GL_FALSE,glm::value_ptr(values[0][0]));}