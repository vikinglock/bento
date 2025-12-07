#include "vulkan.h"

VkDevice device;
VkExtent2D swapchainExtent;
VkRenderPass renderPass;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkQueue graphicsQueue;
VkCommandPool commandPool;

uint32_t currentFrame = 0;

void Bento::setRenderTargets(std::vector<Texture*> targets){setRenderTargets(targets.data(),targets.size());}
void Bento::setRenderTargets(Texture** targets,int count){
}
Texture* Bento::AppTexture = nullptr;

Bento::Bento(){}
void Bento::init(const char* title,int width,int height,int x,int y){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    window = glfwCreateWindow(width,height,"vulkan",nullptr,nullptr);
    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(window,framebufferResizeCallback);

    setWindowPos(glm::vec2(x,y));

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = title;
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "bento";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    
    
    #ifdef DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessengerCreateInfo(debugCreateInfo);
    instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    #else
    instanceCreateInfo.enabledLayerCount = 0;
    #endif
    


    VkResult result = vkCreateInstance(&instanceCreateInfo,nullptr,&instance);
    if(vkCreateInstance(&instanceCreateInfo,nullptr,&instance)!=VK_SUCCESS)
        std::cerr<<"couldn't create vulkan instance"<<std::endl;
    
    if(glfwCreateWindowSurface(instance,window,nullptr,&surface)!=VK_SUCCESS)
        std::cerr<<"couldn't create surface"<<std::endl;



    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);
    if(deviceCount==0)std::cerr<<"couldn't find any gpus with vulkan support"<<std::endl;
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount,devices.data());
    for(VkPhysicalDevice dev:devices){//i <3 range based loops
        /*VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(dev,&deviceProperties);
        vkGetPhysicalDeviceFeatures(dev,&deviceFeatures);
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
        */    physicalDevice = dev;//for the sake of my sanity and my crappy integrated gpu i'm gonna skip any checks here
    }
    if(physicalDevice==VK_NULL_HANDLE)std::cerr<<"couldn't find a suitable gpu.. tough luck"<<std::endl;

    
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();

    
    VAO vao;
    vao.setAttrib(0,3,AttribFormat::Float);
    vao.setAttrib(1,3,AttribFormat::Float);
    shader = new Shader("shaders/shader.vert","shaders/shader.frag",vao);

    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
    
    //finally the good stuff    ^^ tat actually took me 7 hours bro i don't wanna do more setup PLEASe

}

void Bento::focus(){
    glfwFocusWindow(window);
}
void Bento::setShader(Shader* shd){
    shader = shd;
}
void Bento::setClearColor(glm::vec4 color){
    clearColor={{{color.x,color.y,color.z,color.w}}};
}
void Bento::bindTexture(Texture* tex,int index){
    
    /* for(size_t i = 0;i < MAX_FRAMES_IN_FLIGHT;i++){
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex->imageView;
        imageInfo.sampler = tex->sampler;

        VkWriteDescriptorSet descriptorWrites[1];

        //descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //descriptorWrites[0].dstSet = descriptorSets[i];
        //descriptorWrites[0].dstBinding = 0;
        //descriptorWrites[0].dstArrayElement = 0;
        //descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //descriptorWrites[0].descriptorCount = 1;
        //descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 1;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device,1,descriptorWrites,0,nullptr);
    } */
}


#ifdef DEBUG
#pragma region debug
VkResult Bento::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Bento::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
void Bento::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}
void Bento::setupDebugMessenger(){
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(instance,&createInfo,nullptr,&debugMessenger)!=VK_SUCCESS)
        std::cerr<<"failed to set up debug messenger!"<<std::endl;
}
#pragma endregion
#endif

void Bento::createLogicalDevice(){//if it's logical why not call it logical device then hmmmm vulkan devs?
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueFamilyCount,nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueFamilyCount,queueFamilies.data());

    bool foundGraphics=false,foundPresent=false;
    for(int i=0;i<queueFamilies.size();i++){
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            graphicsFamily = i;
            foundGraphics = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,i,surface,&presentSupport);
        if(presentSupport){
            presentFamily = i;
            foundPresent = true;
        }
        if(foundGraphics&&foundPresent)break;
    }


    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily,presentFamily};//bruh

    for(uint32_t queueFamily:uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo deviceCreateInfo{};//dude this is just like metal descriptors
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledLayerCount = 0;

    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    

    if(vkCreateDevice(physicalDevice,&deviceCreateInfo,nullptr,&device)!=VK_SUCCESS)
        std::cerr<<"couldn't create device"<<std::endl;//stdLLendl;

    vkGetDeviceQueue(device,graphicsFamily,0,&graphicsQueue);
    vkGetDeviceQueue(device,presentFamily,0,&presentQueue);
}
void Bento::createSwapchain(){
    
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,surface,&capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,surface,&formatCount,nullptr);
    if(formatCount!=0){
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,surface,&formatCount,formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,surface,&presentModeCount,nullptr);
    if(presentModeCount!=0){
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,surface,&presentModeCount,presentModes.data());
    }
    VkSurfaceFormatKHR format = formats[0];
    for(VkSurfaceFormatKHR availableFormat:formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format = availableFormat;
            break;
        }
    }
    swapchainImageFormat = format.format;

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(VkPresentModeKHR availablePresentMode : presentModes){
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = availablePresentMode;
        }
    }

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapchainExtent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        int minWidth = capabilities.minImageExtent.width, maxWidth = capabilities.maxImageExtent.width;
        int minHeight = capabilities.minImageExtent.height, maxHeight = capabilities.maxImageExtent.height;
        
        swapchainExtent = {
            width<=minWidth?static_cast<uint32_t>(minWidth):width>=maxWidth?static_cast<uint32_t>(maxWidth):static_cast<uint32_t>(width),
            height<=minHeight?static_cast<uint32_t>(minHeight):height>=maxHeight?static_cast<uint32_t>(maxHeight):static_cast<uint32_t>(height),
        };
    }

    uint32_t imageCount = capabilities.minImageCount+1;
    if(capabilities.maxImageCount>0&&imageCount>capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;


    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;

    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = format.format;
    swapchainCreateInfo.imageColorSpace = format.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    
    if(graphicsFamily != presentFamily){
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = {graphicsFamily,presentFamily};
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }else{
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device,&swapchainCreateInfo,nullptr,&swapchain)!=VK_SUCCESS)
        std::cerr<<"couldn't create swapchain"<<std::endl;

    vkGetSwapchainImagesKHR(device,swapchain,&imageCount,nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device,swapchain,&imageCount,swapchainImages.data());
}
void Bento::destroySwapchain(){
    for(VkFramebuffer framebuffer:swapchainFramebuffers)
        vkDestroyFramebuffer(device,framebuffer,nullptr);
    for(VkImageView imageView:swapchainImageViews)
        vkDestroyImageView(device,imageView,nullptr);
    vkDestroySwapchainKHR(device,swapchain,nullptr);
}
void Bento::recreateSwapchain(){//swagchain
    int width = 0,height = 0;
    glfwGetFramebufferSize(window,&width,&height);
    while(width==0||height==0){
        glfwGetFramebufferSize(window,&width,&height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(device);

    destroySwapchain();

    createSwapchain();
    createImageViews();
    createFramebuffers();
}
void Bento::createImageViews(){
    swapchainImageViews.resize(swapchainImages.size());
    for(size_t i=0;i<swapchainImages.size();i++){//hmmmmmmmmmmmmmmmmmmm
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        if(vkCreateImageView(device,&createInfo,nullptr,&swapchainImageViews[i])!=VK_SUCCESS)
            std::cerr<<"couldn't create image view "<<i<<std::endl;
    }
}
void Bento::createRenderPass(){
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(device,&renderPassInfo,nullptr,&renderPass)!=VK_SUCCESS)
        std::cerr<<"couldn't create render pass"<<std::endl;
}
void Bento::createFramebuffers(){
    swapchainFramebuffers.resize(swapchainImageViews.size());
    for(size_t i=0;i<swapchainImageViews.size();i++) {
        VkImageView attachments[] = {swapchainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(device,&framebufferInfo,nullptr,&swapchainFramebuffers[i])!=VK_SUCCESS)
            std::cerr<<"couldn't create framebuffer"<<std::endl;
    }
}
void Bento::createCommandPool(){
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//make it end
    poolInfo.queueFamilyIndex = graphicsFamily;

    if(vkCreateCommandPool(device,&poolInfo,nullptr,&commandPool)!=VK_SUCCESS)
        std::cerr<<"couldn't create command pool"<<std::endl;
}
void Bento::createCommandBuffers(){
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    if(vkAllocateCommandBuffers(device,&allocInfo,commandBuffers.data())!=VK_SUCCESS)
        std::cerr<<"couldn't allocate command buffers"<<std::endl;
}
void Bento::createSyncObjects(){
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for(size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
        if(vkCreateSemaphore(device,&semaphoreInfo,nullptr,&imageAvailableSemaphores[i])!=VK_SUCCESS||
            vkCreateSemaphore(device,&semaphoreInfo,nullptr,&renderFinishedSemaphores[i])!=VK_SUCCESS||
            vkCreateFence(device,&fenceInfo,nullptr,&inFlightFences[i])!=VK_SUCCESS)
            std::cerr<<"couldn't create semaphore "<<i<<std::endl;
    }
}
void Bento::predraw(){
    glfwPollEvents();
}
void Bento::createBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties,VkBuffer& buffer,VkDeviceMemory& bufferMemory){
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device,&bufferInfo,nullptr,&buffer)!=VK_SUCCESS)
        std::cerr<<"couldn't create buffer"<<std::endl;
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device,buffer,&memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice,&memProperties);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    for(uint32_t i=0;i<memProperties.memoryTypeCount;i++)
        if(memRequirements.memoryTypeBits&(1<<i)&&
        (memProperties.memoryTypes[i].propertyFlags&properties)==properties)
            allocInfo.memoryTypeIndex = i;

    if(vkAllocateMemory(device,&allocInfo,nullptr,&bufferMemory)!=VK_SUCCESS)
        std::cerr << "couldn't allocate vertex buffer memory"<<std::endl;
    
    vkBindBufferMemory(device,buffer,bufferMemory,0);
}
void Bento::copyBuffer(VkBuffer srcBuffer,VkBuffer dstBuffer,VkDeviceSize size){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device,&allocInfo,&commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer,&beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue,1,&submitInfo,VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device,commandPool,1,&commandBuffer);
}
void Bento::draw(Primitive primitive){
    vkWaitForFences(device,1,&inFlightFences[currentFrame],VK_TRUE,UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,imageAvailableSemaphores[currentFrame],VK_NULL_HANDLE,&imageIndex);
    if(result==VK_ERROR_OUT_OF_DATE_KHR||framebufferResized){
        framebufferResized = false;
        recreateSwapchain();
        return;
    }else if(result!=VK_SUCCESS&&result!=VK_SUBOPTIMAL_KHR)
        std::cerr<<"couldn't acquire swap chain image"<<std::endl;

    
    vkResetFences(device,1,&inFlightFences[currentFrame]);
    
    vkResetCommandBuffer(commandBuffers[currentFrame],0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(commandBuffers[currentFrame],&beginInfo)!=VK_SUCCESS)
        std::cerr<<"couldn't begin command buffer"<<std::endl;

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};//i want to go home to my metal and opengls
    renderPassBeginInfo.renderArea.extent = swapchainExtent;

    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[currentFrame],&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,shader->graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float)swapchainExtent.height;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = -(float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = swapchainExtent;

    vkCmdSetViewport(commandBuffers[currentFrame],0,1,&viewport);
    vkCmdSetScissor(commandBuffers[currentFrame],0,1,&scissor);

    vkCmdBindPipeline(commandBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,shader->graphicsPipeline);
    std::vector<VkDeviceSize> offsets(vertexBuffers.size());
    vkCmdBindVertexBuffers(commandBuffers[currentFrame],0,vertexBuffers.size(),vertexBuffers.data(),offsets.data());
    vkCmdDraw(commandBuffers[currentFrame],vertexCount,1,0,0);
    
    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    if(vkEndCommandBuffer(commandBuffers[currentFrame])!=VK_SUCCESS)
        std::cerr<<"couldn't record command buffer"<<std::endl;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(vkQueueSubmit(graphicsQueue,1,&submitInfo,inFlightFences[currentFrame])!=VK_SUCCESS)
        std::cout<<"couldn't submit draw command buffer"<<std::endl;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue,&presentInfo);
    
    if(result==VK_ERROR_OUT_OF_DATE_KHR||result==VK_SUBOPTIMAL_KHR||framebufferResized){
        framebufferResized = false;
        recreateSwapchain();
    }else if(result!=VK_SUCCESS)
        std::cerr<<"couldn't present swap chain image"<<std::endl;

    currentFrame = (currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    vertexCount = -1;
}
void Bento::render(){

}
bool Bento::isRunning(){
    return !glfwWindowShouldClose(window);
}
void Bento::setWindowPos(glm::vec2 pos){
    #ifdef WIN32
    pos.y+=GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CXPADDEDBORDER);
    #endif
    glfwSetWindowPos(window,pos.x,pos.y);
}
void Bento::exit(){
    vkDeviceWaitIdle(device);

    delete shader;

    destroySwapchain();
    
    for(VkBuffer buf:vertexBuffers)
        vkDestroyBuffer(device,buf,nullptr);
    for(VkDeviceMemory buf:vertexBufferMemories)
        vkFreeMemory(device,buf,nullptr);
    
    vkDestroyRenderPass(device,renderPass,nullptr);
    
    for(size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
        vkDestroySemaphore(device,imageAvailableSemaphores[i],nullptr);
        vkDestroySemaphore(device,renderFinishedSemaphores[i],nullptr);
        vkDestroyFence(device,inFlightFences[i],nullptr);
    }
    
    vkDestroyCommandPool(device,commandPool,nullptr);
    vkDestroyDevice(device,nullptr);
    #ifdef DEBUG
    DestroyDebugUtilsMessengerEXT(instance,debugMessenger,nullptr);
    #endif
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}


void Bento::startLoop(){
    while(this->isRunning()){
        loop();
    }
    exit();
}
