#pragma once
#include "vulkan.h"

#ifdef CONVERT
#include "../../lib/shaders/glslang/SPIRV/GlslangToSpv.h"
#include "../../lib/shaders/glslang/glslang/Public/ShaderLang.h"
#include "../../lib/shaders/glslang/glslang/Public/ResourceLimits.h"
#endif

class Shader{
    VkShaderModule createShaderModule(std::vector<uint8_t> code){
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        if(vkCreateShaderModule(device,&createInfo,nullptr,&shaderModule)!=VK_SUCCESS)
            std::cerr<<"couldn't create shader module"<<std::endl;
        return shaderModule;
    }
public:
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    
    Shader(std::string vertPath, std::string fragPath,VAO vao = VAO()){
        VkDescriptorSetLayoutBinding uboVertexLayoutBinding{};
        uboVertexLayoutBinding.binding = 0;
        uboVertexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboVertexLayoutBinding.descriptorCount = 1;
        uboVertexLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboVertexLayoutBinding.pImmutableSamplers = nullptr;//i don't remember writing this

        VkDescriptorSetLayoutBinding uboFragmentLayoutBinding{};
        uboFragmentLayoutBinding.binding = 0;
        uboFragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboFragmentLayoutBinding.descriptorCount = 1;
        uboFragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        uboFragmentLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayout descriptorSetLayout;

        VkDescriptorSetLayoutBinding bindings[] = {uboVertexLayoutBinding,uboFragmentLayoutBinding};//samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 2;
        layoutInfo.pBindings = bindings;
        
        if(vkCreateDescriptorSetLayout(device,&layoutInfo,nullptr,&descriptorSetLayout)!=VK_SUCCESS)
            std::cout<<"couldn't create descriptor set layout"<<std::endl;
    

        ///uniform get data stuff things here

        VkDeviceSize bufferSize = sizeof(float)*3;

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            Bento::createBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,uniformBuffers[i],uniformBuffersMemory[i]);
            vkMapMemory(device,uniformBuffersMemory[i],0,bufferSize,0,&uniformBuffersMapped[i]);
        }

        glm::vec3 col = glm::vec3(0.5,0.3,0.8);

        memcpy(uniformBuffersMapped[currentFrame],&col,sizeof(glm::vec3));

        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        /*VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/

        //VkDescriptorPoolSize poolSizes[1];

        //poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        //poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // VkDescriptorPoolCreateInfo poolInfo{};
        // poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        // poolInfo.poolSizeCount = 1;
        // poolInfo.pPoolSizes = poolSizes;
        // poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);


        std::vector<uint8_t> vertShaderCode;
        std::vector<uint8_t> fragShaderCode;
        
        std::string dir = getExecutablePath();
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

        #ifdef CONVERT
            std::ifstream vertFile(vertPath);
            std::ifstream fragFile(fragPath);
            std::string vSource = std::string(std::istreambuf_iterator<char>(vertFile),std::istreambuf_iterator<char>());
            std::string fSource = std::string(std::istreambuf_iterator<char>(fragFile),std::istreambuf_iterator<char>());

            vertShaderCode = Shader::convertShader(vSource,EShLangVertex,vertFilename);
            fragShaderCode = Shader::convertShader(fSource,EShLangFragment,fragFilename);


            /*std::ofstream oVertFile(dir+vertDir+"/cache"+vertFilename+".vs",std::ios::trunc);
            std::ofstream oFragFile(dir+fragDir+"/cache"+fragFilename+".fs",std::ios::trunc);
            if(oVertFile.is_open()){
                oVertFile<<vertShaderCode;
                oVertFile.close();
            }
            if(oFragFile.is_open()){
                oFragFile<<fragShaderCode;
                oFragFile.close();
            }*/
        #else
            vertShaderCode = loadFile(dir+vertDir+"/cache"+vertFilename+".vspv");
            fragShaderCode = loadFile(dir+fragDir+"/cache"+fragFilename+".fspv");
        #endif

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        
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


        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        /*colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,fragShaderStageInfo};


        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        //pipelineLayoutInfo.setLayoutCount = 0;
        //pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;//i see it all kind of translate into metal
        pipelineLayoutInfo.pPushConstantRanges = nullptr;//now i get why translation layers exist

        if(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&pipelineLayout)!=VK_SUCCESS)
            std::cerr<<"couldn't create pipeline layout"<<std::endl;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        
        bindingDescriptions.resize(vao.attributes.size());
        attributeDescriptions.resize(vao.attributes.size());
        for(int i = 0; i < vao.attributes.size(); i++){
            switch(vao.attributes[i].format){
                case AttribFormat::Float:
                    switch(vao.attributes[i].size){
                        case 1:attributeDescriptions[i].format=VK_FORMAT_R32_SFLOAT;bindingDescriptions[i].stride=sizeof(float);break;
                        case 2:attributeDescriptions[i].format=VK_FORMAT_R32G32_SFLOAT;bindingDescriptions[i].stride=sizeof(float)*2;break;
                        case 3:attributeDescriptions[i].format=VK_FORMAT_R32G32B32_SFLOAT;bindingDescriptions[i].stride=sizeof(float)*3;break;
                        case 4:attributeDescriptions[i].format=VK_FORMAT_R32G32B32A32_SFLOAT;bindingDescriptions[i].stride=sizeof(float)*4;break;
                    }
                break;
                case AttribFormat::Int:
                    switch(vao.attributes[i].size){
                        case 1:attributeDescriptions[i].format=VK_FORMAT_R32_SINT;bindingDescriptions[i].stride=sizeof(int);break;
                        case 2:attributeDescriptions[i].format=VK_FORMAT_R32G32_SINT;bindingDescriptions[i].stride=sizeof(int)*2;break;
                        case 3:attributeDescriptions[i].format=VK_FORMAT_R32G32B32_SINT;bindingDescriptions[i].stride=sizeof(int)*3;break;
                        case 4:attributeDescriptions[i].format=VK_FORMAT_R32G32B32A32_SINT;bindingDescriptions[i].stride=sizeof(int)*4;break;
                    }
                break;
                case AttribFormat::UInt:
                    switch(vao.attributes[i].size){
                        case 1:attributeDescriptions[i].format=VK_FORMAT_R32_UINT;bindingDescriptions[i].stride=sizeof(unsigned int);break;
                        case 2:attributeDescriptions[i].format=VK_FORMAT_R32G32_UINT;bindingDescriptions[i].stride=sizeof(unsigned int)*2;break;
                        case 3:attributeDescriptions[i].format=VK_FORMAT_R32G32B32_UINT;bindingDescriptions[i].stride=sizeof(unsigned int)*3;break;
                        case 4:attributeDescriptions[i].format=VK_FORMAT_R32G32B32A32_UINT;bindingDescriptions[i].stride=sizeof(unsigned int)*4;break;
                    }
                break;
                case AttribFormat::Short:
                    switch(vao.attributes[i].size){
                        case 1:attributeDescriptions[i].format=VK_FORMAT_R16_SINT;bindingDescriptions[i].stride=sizeof(short);break;
                        case 2:attributeDescriptions[i].format=VK_FORMAT_R16G16_SINT;bindingDescriptions[i].stride=sizeof(short)*2;break;
                        case 3:attributeDescriptions[i].format=VK_FORMAT_R16G16B16_SINT;bindingDescriptions[i].stride=sizeof(short)*3;break;
                        case 4:attributeDescriptions[i].format=VK_FORMAT_R16G16B16A16_SINT;bindingDescriptions[i].stride=sizeof(short)*4;break;
                    }
                break;
                }
            bindingDescriptions[i].binding = i;
            attributeDescriptions[i].binding = i;
            attributeDescriptions[i].location = i;
            attributeDescriptions[i].offset = vao.attributes[i].offset;
            bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&graphicsPipeline)!=VK_SUCCESS)
            std::cerr<<"couldn't create graphics pipeline"<<std::endl;
            
        vkDestroyShaderModule(device,vertShaderModule,nullptr);
        vkDestroyShaderModule(device,fragShaderModule,nullptr);
    }
    ~Shader(){
        //vkDestroyDescriptorSetLayout(device,descriptorSetLayout,nullptr);
        //vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
        //vkDestroyPipeline(device,graphicsPipeline,nullptr);
        // for(size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++){
        //     vkDestroyBuffer(device,uniformBuffers[i],nullptr);
        //     vkFreeMemory(device,uniformBuffersMemory[i],nullptr);
        // }
        //vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
    
    static std::vector<uint8_t> convertShader(std::string source,EShLanguage lang,std::string name = ""){
        glslang::InitializeProcess();
        glslang::TShader shader(lang);
        
        const char* src = source.c_str();
        shader.setStrings(&src,1);
        shader.setEnvInput(glslang::EShSourceGlsl,lang,glslang::EShClientOpenGL,410);
        shader.setEnvClient(glslang::EShClientVulkan,glslang::EShTargetVulkan_1_1);
        shader.setEnvTarget(glslang::EShTargetSpv,glslang::EShTargetSpv_1_1);

        if(!shader.parse(GetDefaultResources(),410,false,EShMsgDefault))std::cerr<<"couldn't convert shaders\n"<<name<<" error log:\n"<<shader.getInfoLog()<<std::endl;

        glslang::TProgram prog;
        prog.addShader(&shader);
        prog.link(EShMsgDefault);

        std::vector<uint32_t> spirv;
        glslang::GlslangToSpv(*prog.getIntermediate(lang),spirv);
        glslang::FinalizeProcess();
        std::vector<uint8_t> smallspirv((uint8_t*)spirv.data(),(uint8_t*)spirv.data()+spirv.size()*sizeof(uint32_t));
        return smallspirv;
    }
};