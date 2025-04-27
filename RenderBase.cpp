#include "RenderBase.h"

#include "vector"
#include "iostream"
#include "fstream"

#define VOLK_IMPLEMENTATION
#include "volk/volk.h"

std::vector<char> readFile(const std::string& filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}


//=====================================================================
//===============================PIPELINE BUILDER======================
//=====================================================================
void PipelineBuilder::setShader(Context & context, VkShaderStageFlagBits stage, std::string filepath, std::string entrypoint){
    auto shaderCode = readFile(filepath);
    VkShaderModule shaderModule = {};

    VkShaderModuleCreateInfo shaderModuleCI = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        shaderCode.size(),
        reinterpret_cast<const uint32_t*>(shaderCode.data())
    };
    if(vkCreateShaderModule(context.device, &shaderModuleCI, nullptr, &shaderModule) != VK_SUCCESS){
        std::cout << "could not create shader module" << std::endl;
        exit(1);
    }

    VkPipelineShaderStageCreateInfo shaderStageCI = {};
    shaderStageCI = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    //sType
        nullptr,                                                //pNext
        {},                                                     //flags
        stage,                                                  //stage
        shaderModule,                                           //module
        "main",                                                 //pName
        nullptr                                                 //pSpecializationInfo
    };

    this->shaderStages.push_back(shaderStageCI);
}

void PipelineBuilder::setInputAssembly(VkPrimitiveTopology topology){
    //INPUT ASSEMBLY STATE
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        topology,                                                       //topology
        VK_FALSE                                                        //primitiveRestartEnable
    };

    this->inputAssemblyState = inputAssemblyCI;

    return;
}

void PipelineBuilder::setVertexInputState(){
    //VERTEX INPUT STATE
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    this->vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    this->vertexInputState.pNext = nullptr;
    this->vertexInputState.flags = 0;
    this->vertexInputState.vertexBindingDescriptionCount = 1;
    this->vertexInputState.pVertexBindingDescriptions = bindingDescription;
    this->vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>((*attributeDescriptions).size());
    this->vertexInputState.pVertexAttributeDescriptions = (*attributeDescriptions).data();
}

void PipelineBuilder::setTessellationState(){
    //TESSELATION STATE
    VkPipelineTessellationStateCreateInfo tessellationCI = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,      //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        0                                                               //patchControlPoints
    };

    this->tessellationState = tessellationCI;
}

void PipelineBuilder::setViewportState(VkViewport & viewport, VkRect2D & scissor){
    //VIEWPORT STATE
    VkPipelineViewportStateCreateInfo viewportCI = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,          //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        1,                                                              //viewportCount
        &viewport,                                                      //pViewports
        1,                                                              //scissorCount
        &scissor                                                        //pScissors
    };

    this->viewportState = viewportCI;
}

void PipelineBuilder::setRasterizationState(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace, float lineWidth){
    //RASTERIZATION STATE
    VkPipelineRasterizationStateCreateInfo rasterizationCI = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,     //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        VK_FALSE,                                                       //depthClampEnable
        VK_FALSE,                                                       //rasterizerDiscardEnable
        polygonMode,                                                    //polygonMode
        cullMode,                                                       //cullMode
        frontFace,                                                      //frontFace
        VK_FALSE,                                                       //depthBiasEnable
        0.0f,                                                           //depthBiasConstantFactor
        0.0f,                                                           //depthBiasClamp
        0.0f,                                                           //depthBiasSlopeFactor
        lineWidth                                                       //lineWidth
    };

    this->rasterizationState = rasterizationCI;
}

void PipelineBuilder::setMultisampleState(){
    //MULTISAMPLE STATE
    VkPipelineMultisampleStateCreateInfo multisampleCI = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,       //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        VK_SAMPLE_COUNT_1_BIT,                                          //rasterizationSamples
        VK_FALSE,                                                       //sampleShadingEnable
        0.0f,                                                           //minSampleShading
        nullptr,                                                        //pSampleMask
        VK_FALSE,                                                       //alphaToCoverageEnable
        VK_FALSE                                                        //alphaToOneEnable
    };

    this->multisampleState = multisampleCI;
}

void PipelineBuilder::setColorblendState(){
    //COLOR BLEND STATE
    VkPipelineColorBlendAttachmentState * colorBlendAttachment = new VkPipelineColorBlendAttachmentState{
        VK_FALSE,                                                       //blendEnable
        VK_BLEND_FACTOR_ONE,                                            //srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                           //dstColorBlendFactor
        VK_BLEND_OP_ADD,                                                //colorBlendOp
        VK_BLEND_FACTOR_ONE,                                            //srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                           //dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                                //alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |           //colorWriteMask
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendCI = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,       //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        VK_FALSE,                                                       //logicOpEnable
        VK_LOGIC_OP_COPY,                                               //logicOp
        1,                                                              //attachmentCount
        colorBlendAttachment,                                          //pAttachments
                                                                        //blendConstants
    };

    this->colorblendState = colorBlendCI;
}

void PipelineBuilder::setPipelineLayout(Context & context){
    //GRAPHICS PIPELINE AND LAYOUT
    VkPipelineLayoutCreateInfo pipelineLayoutCI = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,                  //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        0,                                                              //setLayoutCount
        nullptr,                                                        //pSetLayouts
        0,                                                              //pushConstantRangeCount
        nullptr                                                         //pPushConstantRanges
    };

    if(vkCreatePipelineLayout(context.device, &pipelineLayoutCI, nullptr, &this->pipelineLayout) != VK_SUCCESS){
        std::cout << "could not create pipeline layout" << std::endl;
        exit(1);
    }
}

VkPipeline & PipelineBuilder::createPipeline(Context & context, RenderPass & renderPass){
    std::cout << "=" << std::endl;
    std::cout << this->vertexInputState.pVertexAttributeDescriptions->binding << std::endl;
    VkGraphicsPipelineCreateInfo graphicsPipelineCI = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,        //sType 
        nullptr,                                                //pNext
        0,                                                      //flags
        this->shaderStages.size(),                                                      //stageCount
        this->shaderStages.data(),                                  //pStages
        &this->vertexInputState,                                    //pVertexInputState
        &this->inputAssemblyState,                                       //pInputAssemblyState
        &this->tessellationState,                                        //pTessellationState
        &this->viewportState,                                            //pViewportState
        &this->rasterizationState,                                       //pRasterizationState
        &this->multisampleState,                                         //pMultisampleState
        nullptr,                                                //pDepthStencilState
        &this->colorblendState,                                          //pColorBlendState
        nullptr,                                                //pDynamicState
        this->pipelineLayout,                                         //layout
        renderPass.renderPass,                                            //renderPass
        0,                                                      //subpass
        nullptr,                                                //basePipelineHandle
        {}                                                      //basePipelineIndex
    };

    if(vkCreateGraphicsPipelines(context.device, nullptr, 1, &graphicsPipelineCI, nullptr, &this->pipeline) != VK_SUCCESS){
        std::cout << "could not create pipeline" << std::endl;
        exit(1);
    }

    return this->pipeline;
}

//=====================================================================
//===============================CONTEXT===============================
//=====================================================================
void Context::createInstance(){
    //==create instance==
    //init volk
    if(volkInitialize() != VK_SUCCESS){
        return;
    } 
    //init sdl
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);

    //get needed instance extensions required by sdl
    uint32_t sdlExtensions = 0;
    const char * const *extensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensions);

    VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,             //sType
        nullptr,                                        //pNext
        "render",                                       //pApplicationName
        0,                                              //applicationVersion
        nullptr,                                        //pEngineName
        0,                                              //engineVersion
        VK_API_VERSION_1_3                              //apiVersion
    };

    VkInstanceCreateInfo instance_ci = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         //sType 
        nullptr,                                        //pNext
        0,                                              //flags
        &app_info,                                      //pApplicationInfo
        0,                                              //enabledLayerCount
        nullptr,                                        //ppEnabledLayerNames
        sdlExtensions,                                  //enabledExtensionCount
        extensions                                      //ppEnabledExtensionNames
    };

    //create instance
    if(vkCreateInstance(&instance_ci, nullptr, &this->instance) != VK_SUCCESS){
        std::cout << "could not create instance" << std::endl;
    }

    //load function pointers from instance
    volkLoadInstance(this->instance);
}

void Context::createPhysicalDevice(){
    //==create physical device==
    //enumerate physical devices
    uint32_t deviceCount = 0;
    
    if(vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr) != VK_SUCCESS){
        std::cout << "could not query physical devices" << std::endl;
        exit(1);
    }
    std::vector<VkPhysicalDevice> queried_devices(deviceCount);
    if(vkEnumeratePhysicalDevices(this->instance, &deviceCount, queried_devices.data()) != VK_SUCCESS){
        std::cout << "could not query physical devices" << std::endl;
        exit(1);
    }

    //pick discrete gpu
    VkPhysicalDeviceProperties deviceProperties = {};
    for(auto device : queried_devices){
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            this->physicalDevice = device;
            break;
        }
    }

    
}

void Context::createLogicalDeviceAndQueue(){
    //query for device queues
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    //find graphics and transfer queue
    int i = 0;
    for(auto queueFamily : queueFamilyProperties){
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_TRANSFER_BIT != 0){
            this->queue.queueFamilyIndex = i;
            this->queue.queueCount = queueFamily.queueCount;
        }
        i++;
    }

    //queue create infos
    std::vector<float> priorities(this->queue.queueCount);
    for(auto priority : priorities){
        priority = 1.0f;
    }
    VkDeviceQueueCreateInfo queueCI = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        this->queue.queueFamilyIndex,
        this->queue.queueCount,
        priorities.data()
    };

    std::vector<const char*> deviceExtensions = {
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo deviceCI = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           //sType
        nullptr,                                        //pNext
        0,                                              //flags
        1,                                              //queueCreateInfoCount
        &queueCI,                                       //pQueueCreateInfos
        0,                                              //enabledLayerCount
        nullptr,                                        //ppEnabledLayerNames
        1,                                              //enabledExtensionCount
        deviceExtensions.data(),                        //ppEnabledExtensionNames
        nullptr                                         //pEnabledFeatures
    };

    //create device
    if(vkCreateDevice(this->physicalDevice, &deviceCI, nullptr, &this->device) != VK_SUCCESS){
        std::cout << "could not create device" << std::endl;
        exit(1);
    }

    //grab queue handle
    vkGetDeviceQueue(this->device, this->queue.queueFamilyIndex, 0, &this->queue.queueFamily);
}

void Context::initContext(){
    this->createInstance();
    this->createPhysicalDevice();
    this->createLogicalDeviceAndQueue();
}

//=====================================================================
//===============================DISPLAY===============================
//=====================================================================
void Display::createWindowAndSurface(Context & context, int width, int height){
    this->window = SDL_CreateWindow("hello", width, height, SDL_WINDOW_VULKAN);
    if(false == SDL_Vulkan_CreateSurface(this->window, context.instance, nullptr, &this->surface)){
        std::cout << SDL_GetError() << std::endl;
    }
}

void Display::createSwapchain(Context & context){
    //query surface capabilities for current extent of surface
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice, this->surface, &surfaceCapabilities);

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.physicalDevice, this->surface, &presentModeCount,nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.physicalDevice, this->surface, &presentModeCount,presentModes.data());

    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, this->surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, this->surface, &surfaceFormatCount, surfaceFormats.data());

    this->swapchainExtent = surfaceCapabilities.currentExtent;

    //create viewport based off surface
    this->viewport.x = 0.0f;
    this->viewport.y = 0.0f;
    this->viewport.height = surfaceCapabilities.currentExtent.height;
    this->viewport.width = surfaceCapabilities.currentExtent.width;
    this->viewport.maxDepth = 1.0f;
    this->viewport.minDepth = 0.0f;
    std::cout<<this->viewport.height<< std::endl;

    VkSwapchainCreateInfoKHR swapchainCI = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,        //sType
        nullptr,                                            //pNext
        {},                                                 //flags
        this->surface,                                      //surface
        2,                                                  //minImageCount
        VK_FORMAT_R8G8B8A8_SRGB,                            //imageFormat
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,                  //imageColorSpace
        surfaceCapabilities.currentExtent,                              //imageExtent
        1,                                                  //imageArrayLayers
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                //imageUsage
        {},                                                 //imageSharingMode
        1,                                                  //queueFamilyIndexCount
        &context.queue.queueFamilyIndex,                    //pQueueFamilyIndices
        surfaceCapabilities.currentTransform,               //preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,                  //compositeAlpha
        VK_PRESENT_MODE_MAILBOX_KHR,                        //presentMode
        VK_FALSE,                                           //clipped
        nullptr                                             //oldSwapchain
    };

    if(vkCreateSwapchainKHR(context.device, &swapchainCI, nullptr, &this->swapchain) != VK_SUCCESS){
        std::cout << "could not create swapchain" << std::endl;
        exit(1);
    }
}

std::vector<Image> Display::getImagesAndViews(Context & context){
    uint32_t swapchainImageCount = 0;
    std::vector<VkImageView> Vulk_imageViews;
    std::vector<VkImage> Vulk_images;
    std::vector<Image> images;

    vkGetSwapchainImagesKHR(context.device, this->swapchain, &swapchainImageCount, nullptr);
    Vulk_images.resize(swapchainImageCount);
    if(vkGetSwapchainImagesKHR(context.device, this->swapchain, &swapchainImageCount, Vulk_images.data()) != VK_SUCCESS){
        std::cout << "could not get swapchain images" << std::endl;
        exit(1);
    }  

    Vulk_imageViews.resize(Vulk_images.size());

    for(int i = 0; i < Vulk_images.size(); i++){

        VkComponentMapping componentMapping = {
            VK_COMPONENT_SWIZZLE_IDENTITY,                      //r
            VK_COMPONENT_SWIZZLE_IDENTITY,                      //g
            VK_COMPONENT_SWIZZLE_IDENTITY,                      //b
            VK_COMPONENT_SWIZZLE_IDENTITY                       //a
        };

        VkImageSubresourceRange imageSubresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,                          //aspectMask
            0,                                                  //baseMipLevel
            1,                                                  //levelCount
            0,                                                  //baseArrayLayer
            1                                                   //layerCount
        };

        VkImageViewCreateInfo imageViewCI = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,           //sType
            nullptr,                                            //pNext
            0,                                                  //flags
            Vulk_images[i],                                              //image
            VK_IMAGE_VIEW_TYPE_2D,                              //viewType
            VK_FORMAT_R8G8B8A8_SRGB,                            //format
            componentMapping,                                   //components
            imageSubresourceRange                               //subresourceRange
        };

        if(vkCreateImageView(context.device, &imageViewCI, nullptr, &Vulk_imageViews[i]) != VK_SUCCESS){
            std::cout << "could not create image view" << std::endl;
            exit(1);
        }

        Image image = {
            Vulk_images[i],
            Vulk_imageViews[i]
        };
        images.push_back(image);
    }
    std::cout << images.size() << std::endl;
    return images;
}

uint32_t Display::getNextPresentableSwapchainIndex(Context & context, Display & display, Semaphore & imageAvailable){
    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(context.device, display.swapchain, UINT64_MAX, imageAvailable.semaphore, nullptr, &imageIndex);
    return imageIndex;
}

void Display::initDisplay(Context & context, int width, int height){
    this->createWindowAndSurface(context, width, height);
    this->createSwapchain(context);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->swapchainExtent;
    this->defaultScissor = scissor;
}

//=====================================================================
//===============================COMMANDBUFFER=========================
//=====================================================================

void CommandBuffer::createPool(Context & context){
    VkCommandPoolCreateInfo commandPoolCI = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        context.queue.queueFamilyIndex
    };

    if(vkCreateCommandPool(context.device, &commandPoolCI, nullptr, &this->pool) != VK_SUCCESS){
        std::cout << "could not create command pool" << std::endl;
        exit(1);
    }
}

void CommandBuffer::allocateBuffer(Context & context, VkCommandPool & pool){
    VkCommandBufferAllocateInfo commandBufferAllocateI = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    if(vkAllocateCommandBuffers(context.device, &commandBufferAllocateI, &this->buffer) != VK_SUCCESS){
        std::cout << "could not create command buffer" << std::endl;
        exit(1);
    }
}

void CommandBuffer::initCommandBuffer(Context & context){
    this->createPool(context);
    this->allocateBuffer(context, this->pool); 
}

void CommandBuffer::initCommandBuffer(Context & context, VkCommandPool & pool){
    this->createPool(context);
    this->allocateBuffer(context, pool); 
}

//=====================================================================
//===============================RENDERPASS============================
//=====================================================================

void RenderPass::createFramebuffers(Context & context, std::vector<Image> images, VkExtent2D & extent){
    this->frameBuffers.resize(images.size());
    std::cout << images.size() << std::endl;

    for(int i = 0; i < images.size(); i++){
    
        VkFramebufferCreateInfo framebufferCI = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,          //sType
            nullptr,                                            //pNext
            0,                                                  //flags
            this->renderPass,                                   //renderPass
            1,                                                  //attachmentCount
            &images[i].imageView,
            extent.width,                                       //width
            extent.height,                                      //height
            1                                                   //layers
        };

        if(vkCreateFramebuffer(context.device, &framebufferCI, nullptr, &this->frameBuffers[i]) != VK_SUCCESS){
            std::cout << "could not create framebuffer" << std::endl;
            exit(1);
        }
    }
}

void RenderPass::initRenderPass(Context & context, CommandBuffer & commandBuffer){
    this->commandBuffer = commandBuffer;

    VkAttachmentDescription * colorAttachment = new VkAttachmentDescription{
        0,                                                      //flags
        VK_FORMAT_R8G8B8A8_SRGB,                                //format
        VK_SAMPLE_COUNT_1_BIT,                                  //samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,                            //loadOp
        VK_ATTACHMENT_STORE_OP_STORE,                           //storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,                        //stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,                       //stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,                              //initialLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                         //finalLayout
    };

    VkAttachmentReference * colorAttachmentRef = new VkAttachmentReference{
        0,                                                      //attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                //layout
    };

    VkSubpassDescription * subpassDescription = new VkSubpassDescription{
        0,                                                      //flags
        VK_PIPELINE_BIND_POINT_GRAPHICS,                        //pipelineBindPoint
        0,                                                      //inputAttachmentCount
        nullptr,                                                //pInputAttachments
        1,                                                      //colorAttachmentCount
        colorAttachmentRef,                                    //pColorAttachments
        nullptr,                                                //pResolveAttachments
        nullptr,                                                //pDepthStencilAttachment
        0,                                                      //preserveAttachmentCount
        nullptr                                                 //pPreserveAttachments

    };

    VkSubpassDependency * dependency = new VkSubpassDependency{
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassCI = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,              //sType
        nullptr,                                                //pNext
        0,                                                      //flags
        1,                                                      //attachmentCount
        colorAttachment,                                       //pAttachments
        1,                                                      //subpassCount
        subpassDescription,                                    //pSubpasses
        1,                                                      //dependencyCount
        dependency                                             //pDependencies
    };
    
    if(vkCreateRenderPass(context.device, &renderPassCI, nullptr, &this->renderPass) != VK_SUCCESS){
        std::cout << "could not create render pass" << std::endl;
    }
}

void RenderPass::setRenderArea(int width, int height){
    //render area
    VkOffset2D offset = {
        0,                                              //x
        0                                               //y
    };

    VkRect2D renderArea = {
        offset,                                         //offset
        {width, height}                                 //extent
    };

    this->renderArea = renderArea;
}

void RenderPass::setClearColor(float color[4]){
    VkClearValue clearColor = {
        {*color}
    };

    this->clearColor = clearColor;
}

void RenderPass::startRenderPass(VkPipeline & pipeline, int imageIndex){
    vkResetCommandBuffer(this->commandBuffer.buffer, 0);
    VkCommandBufferBeginInfo commandBufferBeginCI = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr
    };

    if(vkBeginCommandBuffer(this->commandBuffer.buffer, &commandBufferBeginCI)){
        std::cout << "could not start command buffer" << std::endl;
        exit(1);
    }
    
    VkRenderPassBeginInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        this->renderPass,
        this->frameBuffers[imageIndex],
        this->renderArea,
        1,
        &this->clearColor
    };

    vkCmdBeginRenderPass(this->commandBuffer.buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(this->commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void RenderPass::endRenderPass(){
    vkCmdEndRenderPass(this->commandBuffer.buffer);
    if(vkEndCommandBuffer(this->commandBuffer.buffer) != VK_SUCCESS){
        std::cout << "could not record command buffer" << std::endl;
        exit(1);
    }
}

void RenderPass::drawVertices(VkPipeline pipeline, int vertexCount){
    vkCmdDraw(this->commandBuffer.buffer, vertexCount, 1, 0, 0);
}

void RenderPass::drawIndexed(int indexCount){
    vkCmdDrawIndexed(this->commandBuffer.buffer, indexCount, 1, 0,0,0);
}

void RenderPass::submitWork(Context & context, Semaphore & wait, Semaphore & signal, Fence & fence){
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                      //sType
        nullptr,                                            //pNext
        1,                                                  //waitSemaphoreCount
        &wait.semaphore,                          //pWaitSemaphores
        waitStages,                                         //pWaitDstStageMask
        1,                                                  //commandBufferCount
        &this->commandBuffer.buffer,                                    //pCommandBuffers
        1,                                                  //signalSemaphoreCount
        &signal.semaphore                         //pSignalSemaphores
    };

    if(vkQueueSubmit(context.queue.queueFamily, 1, &submitInfo, fence.fence) != VK_SUCCESS){
        std::cout << "could not submit command buffer to queue" << std::endl;
        exit(1);
    }
}

void RenderPass::submitPresentation(Context & context, Display & display, Semaphore & wait, uint32_t imageIndex){
    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &wait.semaphore,
        1,
        &display.swapchain,
        &imageIndex,
    };

    vkQueuePresentKHR(context.queue.queueFamily, &presentInfo);
}

//=====================================================================
//===============================SEMAPHORE=============================
//=====================================================================

void Semaphore::initSemaphore(Context & context){
    VkSemaphoreCreateInfo semaphoreInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,            //sType
        nullptr                                             //pNext
    };

    if(vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &this->semaphore) != VK_SUCCESS){
        std::cout << "could not create semaphore " << std::endl;
        exit(1);     
    }    
}

//=====================================================================
//===============================FENCE=================================
//=====================================================================

void Fence::initFence(Context & context, bool signaled){
    VkFenceCreateFlagBits flags = {};
    if(signaled){
        flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }else{
        flags = {};
    }
    VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,                //sType
        nullptr,                                            //pNext
        flags                                               //flags
    };

    if(vkCreateFence(context.device, &fenceInfo, nullptr, &this->fence) != VK_SUCCESS){
        std::cout << "could not create fence " << std::endl;
        exit(1);     
    }
}

void Fence::wait(Context & context){
    vkWaitForFences(context.device, 1, &this->fence, VK_TRUE, UINT64_MAX);
}

void Fence::reset(Context & context){
    vkResetFences(context.device, 1, &this->fence);
}

//=====================================================================
//===============================VERTEX================================
//=====================================================================
Vertex::Vertex(glm::vec2 pos, glm::vec3 color){
    this->pos = pos;
    this->color = color;
}
VkVertexInputBindingDescription * Vertex::getBindingDescription(){
            VkVertexInputBindingDescription * bindingDescription = new VkVertexInputBindingDescription;

            bindingDescription->binding = 0;
            bindingDescription->stride = sizeof(Vertex);
            bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> * Vertex::getAttributeDescriptions(){
            std::vector<VkVertexInputAttributeDescription> * attributeDescriptions = new std::vector<VkVertexInputAttributeDescription>(2);

            (*attributeDescriptions)[0].binding = 0;
            (*attributeDescriptions)[0].location = 0;
            (*attributeDescriptions)[0].format = VK_FORMAT_R32G32_SFLOAT;
            (*attributeDescriptions)[0].offset = offsetof(Vertex, pos);

            (*attributeDescriptions)[1].binding = 0;
            (*attributeDescriptions)[1].location = 1;
            (*attributeDescriptions)[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            (*attributeDescriptions)[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
}

//=====================================================================
//===============================BUFFER================================
//=====================================================================
uint32_t Buffer::findMemoryType(Context & context, uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Buffer::init(Context & context, VkDeviceSize size, VkBufferUsageFlagBits bufType, VkSharingMode sharingMode){
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufType;
    bufferInfo.sharingMode = sharingMode;

    this->bufferSize = size;

    if (vkCreateBuffer(context.device, &bufferInfo, nullptr, &this->buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device, this->buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(context, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    if (vkAllocateMemory(context.device, &allocInfo, nullptr, &this->bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(context.device, this->buffer, this->bufferMemory, 0);

}

void Buffer::map(Context & context, void * data){
    //map
    vkMapMemory(context.device, this->bufferMemory, 0, this->bufferSize, 0, &this->mappedMemory);
        memcpy(this->mappedMemory, data, (size_t) this->bufferSize);
}
//=====================================================================
//===============================VERTEXBUFFER==========================
//=====================================================================

void VertexBuffer::init(Context & context, std::vector<Vertex> vertices){
    this->buffer.init(context, sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    this->buffer.map(context, vertices.data());
}

void VertexBuffer::bind(CommandBuffer & cmdBuf){
    VkBuffer vertexBuffers[] = {this->buffer.getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuf.buffer, 0, 1, vertexBuffers, offsets);
}

//=====================================================================
//===============================INDEXBUFFER==========================
//=====================================================================

void IndexBuffer::init(Context & context, std::vector<uint16_t> indices){
    this->buffer.init(context, sizeof(indices[0]) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    this->buffer.map(context, indices.data());
}

void IndexBuffer::bind(CommandBuffer & cmdBuf){
    vkCmdBindIndexBuffer(cmdBuf.buffer, this->buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16);
}