#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#ifdef _WIN32
#include <windows.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <chrono>

#include <stdexcept>
#include <cstdlib>

#include "../../utils.h"
#include "vulkancommon.h"

#include "../vao.h"

#include "texture.h"

#include "../features.h"

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"

#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "../lib/miniaudio/miniaudio.h"
#include "../sound/soundcommon.h"


#ifdef CONVERT
#include "../../lib/shaders/glslang/SPIRV/GlslangToSpv.h"
#include "../../lib/shaders/glslang/glslang/Public/ShaderLang.h"
#include "../../lib/shaders/glslang/glslang/Public/ResourceLimits.h"
#endif

//vulkan really is the final boss of programming
//i don't like vulkan

//i'm just gonna passively work on it beacuse it's not needed but it's still good to have

enum class Primitive{
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip
};

class Shader;

void loop();

class Bento{
public:
    static void framebufferResizeCallback(GLFWwindow*window,int width,int height){
        Bento* bento = reinterpret_cast<Bento*>(glfwGetWindowUserPointer(window));
        bento->framebufferResized = true;
    }

    Bento();
    void init(const char* title,int width,int height,int x=0,int y=0);
    void focus();
    void setShader(Shader* shader);
    void setClearColor(glm::vec4 color);
    template <typename T>
    void setUniform(std::string name,T data,bool onvertex = false){

    }
    void bindTexture(Texture* tex,int index);


    #ifdef DEBUG
    #pragma region debug

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkDebugUtilsMessengerEXT debugMessenger;
    void setupDebugMessenger();
    #pragma endregion
    #endif

    void createLogicalDevice();
    void createSwapchain();
    void destroySwapchain();
    void recreateSwapchain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void predraw();

    std::vector<VkBuffer> vertexBuffers;
    std::vector<VkDeviceMemory> vertexBufferMemories;

    
    static void createBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties,VkBuffer& buffer,VkDeviceMemory& bufferMemory);
    
    template <typename T>
    void setVertexBuffer(int index,std::vector<T> data){
        if(vertexBuffers.size()<=index){
            vertexBuffers.resize(index+1);
            vertexBufferMemories.resize(index+1);//it does kinda feel like i'm straight up just writing metal
        }

        VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Bento::createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer,stagingBufferMemory);
        
        void* dat;
        vkMapMemory(device,stagingBufferMemory,0,bufferSize,0,&dat);
        memcpy(dat,data.data(),(size_t)bufferSize);
        vkUnmapMemory(device,stagingBufferMemory);

        Bento::createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,vertexBuffers[index],vertexBufferMemories[index]);
        copyBuffer(stagingBuffer,vertexBuffers[index],bufferSize);
        
        vkDestroyBuffer(device,stagingBuffer,nullptr);
        vkFreeMemory(device,stagingBufferMemory,nullptr);

        vertexCount = vertexCount<data.size()?vertexCount:data.size();
    }

    void copyBuffer(VkBuffer srcBuffer,VkBuffer dstBuffer,VkDeviceSize size);

    uint32_t vertexCount;
    void startLoop();
    void draw(Primitive primitive);
    void render();
    bool isRunning();
    void setWindowPos(glm::vec2 pos);
    void exit();
    
    void setRenderTargets(Texture** targets,int count);
    void setRenderTargets(std::vector<Texture*> targets);

    static Texture* FrameBuffer;

    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    
    bool framebufferResized = false;

    uint32_t graphicsFamily;
    uint32_t presentFamily;
    
    Shader* shader;

    VkClearValue clearColor;
};

#include "shader.h"
