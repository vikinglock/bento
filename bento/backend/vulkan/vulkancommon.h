#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

extern VkDevice device;
extern VkExtent2D swapchainExtent;
extern VkRenderPass renderPass;
extern VkPhysicalDevice physicalDevice;
extern VkQueue graphicsQueue;
extern VkCommandPool commandPool;
extern uint32_t currentFrame;
const int MAX_FRAMES_IN_FLIGHT = 2;