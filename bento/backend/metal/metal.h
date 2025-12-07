#pragma once

#define MAX_LIGHTS 50

#include <iostream>
#include <unordered_map>
#include <string>
#include <regex>
#include <map>
#include <simd/simd.h>
#include <chrono>

#include <thread>//trio of terror
#include <mutex>
#include <atomic>

#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/gtc/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"
#include "texture.h"

#include "../../features.h"

#include "../file/file.h"


#ifdef IMGUI
#include "../../lib/imgui/imgui.h"
#include "../../lib/imgui/backends/imgui_impl_metal.h"
#include "../../lib/imgui/backends/imgui_impl_osx.h"
#endif

#include "../../lib/miniaudio/miniaudio.h"
#include "../sound/soundcommon.h"


#import <Metal/Metal.h>
#include "metalcommon.h"

class Bento;

#include "enums.h"


#include "shader.h"
#include "framebuffer.h"


const int MAX_FRAMES_IN_FLIGHT = 2;

struct TouchPoint{
    bool down;
    glm::vec2 position;
    int id;
};

class Bento {
    public:
    Bento(const char *title,int w,int h,int x=0,int y=0);
    void initSound();
    void setClearColor(glm::vec4 col = glm::vec4(0,0,0,1));
    void predraw();
    void draw(Primitive primitive);
    void render();
    bool isRunning();
    void setVertices(const std::vector<glm::vec3>& vertices);
    void setNormals(const std::vector<glm::vec3>& normals);
    void setUvs(const std::vector<glm::vec2>& uvs);
    template<typename T>
    void setVertexBuffer(int index,std::vector<T> buf){internalSetVertexBuffer(index,buf.data(),buf.size(),sizeof(T));}
    void setModelMatrix(const glm::mat4 m);
    void setViewMatrix(const glm::mat4 v,const glm::vec3 p);
    void setProjectionMatrix(const glm::mat4 p);
    glm::vec2 getWindowSize();
    glm::vec2 getWindowPos();
    void setWindowPos(glm::vec2 pos);
    void toggleFullscreen();
    void focus();
    bool getKey(int key);
    bool getMouse(int mouse);
    bool getKeyUp(int key);
    bool getKeyDown(int key);
    bool getMouseUp(int mouse);
    bool getMouseDown(int mouse);
    double getScroll(int wheel);
    void setMouseCursor(bool hide,int cursor);
    void setMousePosition(glm::vec2 pos,bool needsFocus=false);
    glm::vec2 getMousePosition();
    glm::vec2 getMouseDelta();
    void lockMouse(bool locked);
    glm::vec2 getControllerAxis(int controller,JoystickType joystick);
    bool getControllerButton(int controller,ButtonType button);
    bool isWindowFocused();
    glm::vec2 getDisplaySize();
    glm::vec2 getFramebufferSize();
    void bindTexture(class Texture *tex,int index);
    void unbindTexture();

    void drawScreen(Shader* shd = Bento::DefaultShader);
    void bindFrameBuffer(FrameBuffer* framebuffer);
    static Texture* AppTexture;
    static FrameBuffer* DefaultFrameBuffer;
    static Shader* DefaultShader;

    void setActiveTextures(int start, int end);
    void setActiveTextures(int ind);
    void setActiveDepthTexture(int ind);
    void setActiveAttachments(int start, int end);
    void setActiveAttachments(int ind);
    void predrawTex(int width,int height);
    void drawTex(Primitive primitive);
    void renderTex();
    void renderToTex(Texture*& tex1,int ind);
    void renderDepthToTex(Texture*& tex,int ind);
    void setShader(Shader* shader);
    void enable(Feature f,bool enabled = true);

    void normalizeTexture(int index, bool normalized);


    template<typename T>
    void setUniform(std::string uniformName,T buf,bool onvertex=false){
        if(onvertex)memcpy((uint8_t*)(shader->vertBuffer).contents+shader->uniformMapVert[uniformName],&buf,shader->sizeMapVert[uniformName]);
        else        memcpy((uint8_t*)(shader->fragBuffer).contents+shader->uniformMapFrag[uniformName],&buf,shader->sizeMapFrag[uniformName]);
    }
    template<typename T>
    void setUniform(std::string uniformName,std::vector<T> buf,bool onvertex=false){
        if(onvertex)memcpy((uint8_t*)(shader->vertBuffer).contents+shader->uniformMapVert[uniformName],buf.data(),shader->sizeMapVert[uniformName]);
        else        memcpy((uint8_t*)(shader->fragBuffer).contents+shader->uniformMapFrag[uniformName],buf.data(),shader->sizeMapFrag[uniformName]);
    }
    template<typename T>
    void setUniform(std::string uniformName,T* buf,bool onvertex=false){
        if(onvertex)memcpy((uint8_t*)(shader->vertBuffer).contents+shader->uniformMapVert[uniformName],buf,shader->sizeMapVert[uniformName]);
        else        memcpy((uint8_t*)(shader->fragBuffer).contents+shader->uniformMapFrag[uniformName],buf,shader->sizeMapFrag[uniformName]);
    }

    void setKeyCallback(void (*callback)(char,KeyCode));
    void setResizeCallback(void (*resizeCallback)(glm::ivec2));
    
    void exit();

    //imgui


    #ifdef IMGUI
    void initImgui();
    void imguiNewFrame();
    void imguiRender();
    #endif

    void startLoop();

    #ifdef IOS
    glm::vec2 getTouchPos(int index);
    bool getTouch(int index);
    bool getTouchDown(int index);
    bool getTouchUp(int index);
    #endif

    //debug

    std::string getFramework(){return "Metal";}//ASS code
    std::string getOperatingSystem(){return "Macos";}


    void poll();
private:
    Shader* shader;

    void internalSetVertexBuffer(int index,void* data,size_t size,size_t typeSize);
    static void (*keyCallback)(char,KeyCode);
    static void (*resizeCallback)(glm::ivec2);


    std::map<int,int> keyStates;
    std::map<int,int> prevKeyStates;
    std::map<int,int> mouseStates;
    std::map<int,int> prevMouseStates;

    static std::vector<TouchPoint> touches;
    static std::vector<TouchPoint> prevTouches;

    glm::vec4 clearColor = glm::vec4(0.0,0.0,0.0,1.0);
    FrameBuffer* framebuffer;
    bool encoding = false;


    NSWindow* window;
    NSApplication* app;
    CAMetalLayer* metalLayer;
    
    id<MTLRenderCommandEncoder> commandEncoder;
    id<MTLCommandBuffer> commandBuffer;

    id<MTLRenderPipelineState> pipelineState;

    id<MTLDepthStencilState> depthStencilState;
    id<MTLDepthStencilState> noDepthStencilState;
    id<MTLDepthStencilState> yesDepthStencilState;

    id<MTLTexture> appTexture;
    std::mutex resizingLock;

    bool shouldClose;
    bool fullscreenable;
    bool focused;

    NSRect windowFrame;

    NSMutableArray *controllers;
    id<CAMetalDrawable> drawable;
    MTLRenderPassDescriptor *passDescriptor;
    id<MTLTexture> depthTexture;
    id<MTLTexture> depthTTexture;
    id<MTLSamplerState> rTSampler;
    int lastWidth;
    int lastHeight;
    glm::vec3 pos;
    glm::vec2 mouseDelta;
    double wheelX;
    double wheelY;
    NSInteger vertCount;
    NSInteger normCount;
    NSInteger uvCount;
};
#ifdef IOS
#import <MetalKit/MetalKit.h>
@interface BentoRenderer : UIResponder <UIApplicationDelegate, MTKViewDelegate>
@property (nonatomic, strong) UIApplication* app;
@property (nonatomic, strong) UIWindow* window;
@property (nonatomic, strong) MTKView* view;
#else
@interface BentoRenderer : NSResponder
@property (nonatomic, strong) NSWindow* window;
#endif
@property (nonatomic, strong) NSMutableArray *controllers;
@property (nonatomic, strong) id<CAMetalDrawable> drawable;
@property (nonatomic, strong) MTLRenderPassDescriptor *passDescriptor;
@property (nonatomic, strong) id<MTLTexture> depthTexture;
@property (nonatomic, strong) id<MTLTexture> depthTTexture;
@property (nonatomic, strong) id<MTLSamplerState> rTSampler;
@property (nonatomic) int lastWidth;
@property (nonatomic) int lastHeight;
@property (nonatomic) glm::vec3 pos;
@property (nonatomic) glm::vec2 mouseDelta;
@property (nonatomic) double wheelX;
@property (nonatomic) double wheelY;
@property (nonatomic) NSInteger vertCount;
@property (nonatomic) NSInteger normCount;
@property (nonatomic) NSInteger uvCount;
@end