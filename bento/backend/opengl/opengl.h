#pragma once

#define MAX_LIGHTS 50

#include <iostream>

#ifdef IMGUI
#include "../../lib/imgui/imgui.h"
#include "../../lib/imgui/backends/imgui_impl_glfw.h"
#include "../../lib/imgui/backends/imgui_impl_opengl3.h"
#endif

#include "../../features.h"

#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/gtc/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"
#include "../../lib/glad/glad.h"
#include "../../lib/GLFW/glfw3.h"

#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <regex>
#include <unordered_map>
#include <unordered_set>

#include <thread>
#include <mutex>
#include <atomic>

#include "../../lib/miniaudio/miniaudio.h"
#include "../sound/soundcommon.h"
#include "openglcommon.h"

#include "texture.h"
#include "framebuffer.h"

class Bento;

enum KeyCode{
    //  #####     KEYS     #####
    KEY_F1 = GLFW_KEY_F1,KEY_F2 = GLFW_KEY_F2,KEY_F3 = GLFW_KEY_F3,KEY_F4 = GLFW_KEY_F4,
    KEY_F5 = GLFW_KEY_F5,KEY_F6 = GLFW_KEY_F6,KEY_F7 = GLFW_KEY_F7,KEY_F8 = GLFW_KEY_F8,
    KEY_F9 = GLFW_KEY_F9,KEY_F10 = GLFW_KEY_F10,KEY_F11 = GLFW_KEY_F11,KEY_F12 = GLFW_KEY_F12,
    KEY_NUM_LOCK = GLFW_KEY_NUM_LOCK,KEY_NUMPAD_0 = GLFW_KEY_KP_0,KEY_NUMPAD_1 = GLFW_KEY_KP_1,
    KEY_NUMPAD_2 = GLFW_KEY_KP_2,KEY_NUMPAD_3 = GLFW_KEY_KP_3,KEY_NUMPAD_4 = GLFW_KEY_KP_4,
    KEY_NUMPAD_5 = GLFW_KEY_KP_5,KEY_NUMPAD_6 = GLFW_KEY_KP_6,KEY_NUMPAD_7 = GLFW_KEY_KP_7,
    KEY_NUMPAD_8 = GLFW_KEY_KP_8,KEY_NUMPAD_9 = GLFW_KEY_KP_9,KEY_NUMPAD_ADD = GLFW_KEY_KP_ADD,
    KEY_NUMPAD_SUBTRACT = GLFW_KEY_KP_SUBTRACT,KEY_NUMPAD_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
    KEY_NUMPAD_DIVIDE = GLFW_KEY_KP_DIVIDE,KEY_NUMPAD_ENTER = GLFW_KEY_KP_ENTER,
    KEY_NUMPAD_DECIMAL = GLFW_KEY_KP_DECIMAL,
    KEY_NUMPAD_EQUALS = GLFW_KEY_KP_EQUAL,
    KEY_NUMPAD_CLEAR = GLFW_KEY_NUM_LOCK,
    KEY_A = GLFW_KEY_A,KEY_B = GLFW_KEY_B,KEY_C = GLFW_KEY_C,KEY_D = GLFW_KEY_D,
    KEY_E = GLFW_KEY_E,KEY_F = GLFW_KEY_F,KEY_G = GLFW_KEY_G,KEY_H = GLFW_KEY_H,
    KEY_I = GLFW_KEY_I,KEY_J = GLFW_KEY_J,KEY_K = GLFW_KEY_K,KEY_L = GLFW_KEY_L,
    KEY_M = GLFW_KEY_M,KEY_N = GLFW_KEY_N,KEY_O = GLFW_KEY_O,KEY_P = GLFW_KEY_P,
    KEY_Q = GLFW_KEY_Q,KEY_R = GLFW_KEY_R,KEY_S = GLFW_KEY_S,KEY_T = GLFW_KEY_T,
    KEY_U = GLFW_KEY_U,KEY_V = GLFW_KEY_V,KEY_W = GLFW_KEY_W,KEY_X = GLFW_KEY_X,
    KEY_Y = GLFW_KEY_Y,KEY_Z = GLFW_KEY_Z,
    KEY_0 = GLFW_KEY_0,KEY_1 = GLFW_KEY_1,KEY_2 = GLFW_KEY_2,KEY_3 = GLFW_KEY_3,
    KEY_4 = GLFW_KEY_4,KEY_5 = GLFW_KEY_5,KEY_6 = GLFW_KEY_6,KEY_7 = GLFW_KEY_7,
    KEY_8 = GLFW_KEY_8,KEY_9 = GLFW_KEY_9,
    KEY_SPACE = GLFW_KEY_SPACE,KEY_TAB = GLFW_KEY_TAB,KEY_RETURN = GLFW_KEY_ENTER,
    KEY_DELETE = GLFW_KEY_DELETE,KEY_ESCAPE = GLFW_KEY_ESCAPE,KEY_BACKSPACE = GLFW_KEY_BACKSPACE,
    KEY_MINUS = GLFW_KEY_MINUS,
    KEY_EQUALS = GLFW_KEY_EQUAL,
    KEY_LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
    KEY_BACKSLASH = GLFW_KEY_BACKSLASH,
    KEY_SEMICOLON = GLFW_KEY_SEMICOLON,
    KEY_QUOTE = GLFW_KEY_APOSTROPHE,
    KEY_COMMA = GLFW_KEY_COMMA,
    KEY_GRAVE = GLFW_KEY_GRAVE_ACCENT,
    KEY_PERIOD = GLFW_KEY_PERIOD,
    KEY_SLASH = GLFW_KEY_SLASH,
    KEY_VOLUME_UP = -1,KEY_VOLUME_DOWN = -1,KEY_MUTE = -1,
    KEY_HELP = GLFW_KEY_INSERT,KEY_INSERT = GLFW_KEY_INSERT,KEY_HOME = GLFW_KEY_HOME,
    KEY_END = GLFW_KEY_END,KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN,KEY_PAGE_UP = GLFW_KEY_PAGE_UP,
    KEY_UP = GLFW_KEY_UP,KEY_DOWN = GLFW_KEY_DOWN,KEY_LEFT = GLFW_KEY_LEFT,KEY_RIGHT = GLFW_KEY_RIGHT,
    KEY_FUNCTION = -1,KEY_CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
    KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,KEY_LEFT_OPTION = GLFW_KEY_LEFT_ALT,KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT,KEY_LEFT_COMMAND = 343,
    KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,KEY_RIGHT_OPTION = GLFW_KEY_RIGHT_ALT,KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT,KEY_RIGHT_COMMAND = 347,
    //  #####     MOUSE BUTTONS     #####
    MOUSE_LEFT = GLFW_MOUSE_BUTTON_1,
    MOUSE_RIGHT = GLFW_MOUSE_BUTTON_2,
    MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_3,
    MOUSE_1 = GLFW_MOUSE_BUTTON_1,
    MOUSE_2 = GLFW_MOUSE_BUTTON_2,
    MOUSE_3 = GLFW_MOUSE_BUTTON_3,
    MOUSE_4 = GLFW_MOUSE_BUTTON_4,
    MOUSE_5 = GLFW_MOUSE_BUTTON_5,
};

enum JoystickType{
    GAMEPAD_JOYSTICK_LEFT,
    GAMEPAD_JOYSTICK_RIGHT,
};
enum ButtonType{
    GAMEPAD_KEY_A = GLFW_GAMEPAD_BUTTON_A,
    GAMEPAD_KEY_B = GLFW_GAMEPAD_BUTTON_B,
    GAMEPAD_KEY_X = GLFW_GAMEPAD_BUTTON_X,
    GAMEPAD_KEY_Y = GLFW_GAMEPAD_BUTTON_Y,

    GAMEPAD_KEY_R1 = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
    GAMEPAD_KEY_R2 = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER+2,
    GAMEPAD_KEY_R3 = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,

    GAMEPAD_KEY_L1 = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
    GAMEPAD_KEY_L2 = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER+2,
    GAMEPAD_KEY_L3 = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,

    GAMEPAD_KEY_START = GLFW_GAMEPAD_BUTTON_START,
    GAMEPAD_KEY_SELECT = GLFW_GAMEPAD_BUTTON_BACK,
    GAMEPAD_KEY_HOME = GLFW_GAMEPAD_BUTTON_GUIDE,
    GAMEPAD_KEY_SCREENSHOT = GLFW_GAMEPAD_BUTTON_GUIDE,

    GAMEPAD_KEY_UP = GLFW_GAMEPAD_BUTTON_DPAD_UP,
    GAMEPAD_KEY_DOWN = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
    GAMEPAD_KEY_LEFT = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
    GAMEPAD_KEY_RIGHT = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
};
enum class Primitive{
    Points = GL_POINTS,
    Lines = GL_LINES,
    LineStrip = GL_LINE_STRIP,
    Triangles = GL_TRIANGLES,
    TriangleStrip = GL_TRIANGLE_STRIP,
};

#include "shader.h"

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
    void setVertexBuffer(int index,std::vector<T> buf){internalSetVertexBuffer(index,buf.data(),buf.size(),sizeof(T));}//i would much rather do what i did for metal than do .tpp bullshit again
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
    void setMousePosition(glm::vec2 pos,bool needsFocus = false);
    glm::vec2 getMousePosition();
    glm::vec2 getMouseDelta();
    glm::vec2 getControllerAxis(int controller,JoystickType joystick);
    bool getControllerButton(int controller,ButtonType button);
    bool isWindowFocused();
    glm::vec2 getDisplaySize();
    glm::vec2 getFramebufferSize();
    void bindTexture(class Texture* tex,int index);
    void unbindTexture();
    void lockMouse(bool locked);

    void drawScreen(Shader* shd = Bento::DefaultShader);
    void bindFrameBuffer(FrameBuffer* framebuffer);
    static Texture* AppTexture;
    static FrameBuffer* DefaultFrameBuffer;
    static Shader* DefaultShader;
    
    void setActiveTextures(int start,int end);
    void setActiveTextures(int ind);
    void setActiveDepthTexture(int ind);
    void setActiveAttachments(int start,int end);
    void setActiveAttachments(int ind);
    void predrawTex(int width,int height);
    void drawTex();
    void renderTex();
    void renderToTex(Texture*& tex1,int ind);
    void renderDepthToTex(Texture*& tex,int ind);
    void renderToTex(Texture*& tex1,Texture*& tex2,int ind);
    void renderToTex(Texture*& tex1,Texture*& tex2,Texture*& tex3,int ind);
    void setShader(Shader* shader);
    void enable(Feature f,bool enabled = true);

    void normalizeTexture(int index,bool normalized);


    void setUniform(std::string uniformName,float value,bool onVertex = false);
    void setUniform(std::string uniformName,int value,bool onVertex = false);
    void setUniform(std::string uniformName,glm::vec2 value,bool onVertex = false);
    void setUniform(std::string uniformName,glm::vec3 value,bool onVertex = false);
    void setUniform(std::string uniformName,glm::mat4 value,bool onVertex = false);
    void setUniform(std::string uniformName,glm::vec4 value,bool onVertex = false);

    void setUniform(std::string uniformName,float* values,int count,bool onVertex = false);
    void setUniform(std::string uniformName,int* values,int count,bool onVertex = false);
    void setUniform(std::string uniformName,glm::vec2* values,int count,bool onVertex = false);
    void setUniform(std::string uniformName,glm::vec3* values,int count,bool onVertex = false);
    void setUniform(std::string uniformName,glm::vec4* values,int count,bool onVertex = false);
    void setUniform(std::string uniformName,glm::mat4* values,int count,bool onVertex = false);
    
    void setUniform(std::string uniformName,const std::vector<float>& values,bool onVertex = false);
    void setUniform(std::string uniformName,const std::vector<int>& values,bool onVertex = false);
    void setUniform(std::string uniformName,const std::vector<glm::vec2>& values,bool onVertex = false);
    void setUniform(std::string uniformName,const std::vector<glm::vec3>& values,bool onVertex = false);
    void setUniform(std::string uniformName,const std::vector<glm::vec4>& values,bool onVertex = false);
    void setUniform(std::string uniformName,const std::vector<glm::mat4>& values,bool onVertex = false);

    void exit();

    void poll();
    //imgui

    #ifdef IMGUI
    void initImgui();
    void imguiNewFrame();
    void imguiRender();
    #endif

    void startLoop();//apple why can't you just be normal

    glm::vec2 getTouchPos(int index);
    bool getTouch(int index);
    bool getTouchDown(int index);
    bool getTouchUp(int index);

    //lights

    void addLight(const glm::vec3 position,const glm::vec3 ambient = glm::vec3(1.0f),const glm::vec3 diffuse = glm::vec3(1.0f),const glm::vec3 specular = glm::vec3(1.0f),float constant = 1.0f,float linear = 0.09f,float quadratic = 0.032f);
    void setLightPos(int index,glm::vec3 position);
    void setLightConstants(int index,float constant);
    void setLightLinears(int index,float linear);
    void setLightQuads(int index,float quad);
    void setLightAmbients(int index,glm::vec3 ambient);
    void setLightDiffuses(int index,glm::vec3 diffuse);
    void setLightSpeculars(int index,glm::vec3 specular);

    void setAmbientColor(glm::vec3 ambient);

    void setKeyCallback(void (*callback)(char,KeyCode));
    void setResizeCallback(void (*callback)(glm::ivec2));

    //debug

    std::string getFramework();
    std::string getOperatingSystem();

    Shader* shader;//no,no it is not.

private:
    std::unordered_set<GLuint> vaos;
    std::unordered_set<GLuint> buffs;

    int vertexAmount;
    int numLights;
    int startAtt,endAtt,startRT,endRT,dTInd;
    glm::vec3 positions[MAX_LIGHTS];
    float constants[MAX_LIGHTS];
    float linears[MAX_LIGHTS];
    float quads[MAX_LIGHTS];
    glm::vec3 ambients[MAX_LIGHTS];
    glm::vec3 diffuses[MAX_LIGHTS];
    glm::vec3 speculars[MAX_LIGHTS];


    std::unordered_map<int,bool> prevKeyStates;
    std::unordered_map<int,bool> prevMouseStates;
    
    GLuint vao,vertexBuffer,normalBuffer,uvBuffer;//,ubo,uboIndex; me when macos (they just don't work idk why)
                                                             // i'll redo it as soon as they make windows more fun to work on (or i guess i could just use linux)

    FrameBuffer* framebuffer;
    GLuint depthRTexture;
    std::vector<GLuint> texture,depthTexture;
    GLuint fbo,screenTex,rbo;
    GLint tvp[4];
    float fboWidth,fboHeight;
    GLuint qvao,qvertexBuffer,quvBuffer;
    
    glm::vec2 mouseDelta = glm::vec2(0);
    bool mouseLocked = false;
    glm::vec2 lockMousePos = glm::vec2(0);


    static void (*keyCallback)(char,KeyCode);
    static void (*resizeCallback)(glm::ivec2);

    static char uppercase(char c) {
        if (c >= 'a' && c <= 'z') return c - 32;
        switch (c) {
            case '1': return '!';
            case '2': return '@';
            case '3': return '#';
            case '4': return '$';
            case '5': return '%';
            case '6': return '^';
            case '7': return '&';
            case '8': return '*';
            case '9': return '(';
            case '0': return ')';
            case '-': return '_';
            case '=': return '+';
            case '[': return '{';
            case ']': return '}';
            case '\\': return '|';
            case ';': return ':';
            case '\'': return '"';
            case ',': return '<';
            case '.': return '>';
            case '/': return '?';
            case '`': return '~';
            default: return c;
        }
    }
    static bool isNumpadKey(KeyCode key) {
        switch(key) {
            case KEY_NUMPAD_0:
            case KEY_NUMPAD_1: case KEY_NUMPAD_2: case KEY_NUMPAD_3:
            case KEY_NUMPAD_4: case KEY_NUMPAD_5: case KEY_NUMPAD_6:
            case KEY_NUMPAD_7: case KEY_NUMPAD_8: case KEY_NUMPAD_9:
            case KEY_NUMPAD_ADD:
            case KEY_NUMPAD_SUBTRACT:
            case KEY_NUMPAD_MULTIPLY:
            case KEY_NUMPAD_DIVIDE: 
            case KEY_NUMPAD_DECIMAL:
            case KEY_NUMPAD_EQUALS:
                return true;
            default:
                return false;
        }
    }
    static bool isSpecialKey(KeyCode key) {
        switch(key) {
            case KEY_LEFT_SHIFT:
            case KEY_LEFT_CONTROL:
            case KEY_LEFT_ALT:
            case KEY_RIGHT_SHIFT:
            case KEY_RIGHT_CONTROL:
            case KEY_RIGHT_ALT:
            case KEY_LEFT_COMMAND:
            case KEY_RIGHT_COMMAND:
                return true;
            default:
                return false;
        }
    }
    static void keyCallbackLoc(GLFWwindow* window,int key,int scancode,int action,int mods){
        #ifdef IMGUI
        ImGui_ImplGlfw_KeyCallback(window,key,scancode,action,mods);
        #endif
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(!isSpecialKey((KeyCode)key)){//i'm (no longer) sorry (this was much worse before (they're all ints so it works in a switch))
                char c = 0;
                const char* name = glfwGetKeyName(key,scancode);

                if(name){
                    c = name[0];
                    if(mods & GLFW_MOD_SHIFT && !isNumpadKey((KeyCode)key))c = uppercase(c);
                }
                if(key == KEY_SPACE)c = 32;
                if(keyCallback)keyCallback(c,(KeyCode)key);
            }
        }
    }
    void internalSetVertexBuffer(int index,void* data,size_t size,size_t typeSize);

    
    GLFWwindow* window;

    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;
    GLuint positionLoc;

    GLuint positionsLoc;
    GLuint constantsLoc;
    GLuint linearsLoc;
    GLuint quadsLoc;
    GLuint ambientsLoc;
    GLuint diffusesLoc;
    GLuint specularsLoc;
    GLuint numLightsLoc;

    GLuint ambientLoc;

    glm::vec3 amb;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    glm::vec3 pos;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
};