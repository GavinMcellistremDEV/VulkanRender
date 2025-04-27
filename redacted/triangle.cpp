RenderBase::RenderBase(){

}

void RenderBase::init(){
    createInstance();
    createWindow();
    createDevice();
    createSwapchain();
    getSwapchainImages();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffer();
    createSyncObjects();
}

void RenderBase::mainLoop(){
    bool running = true;
    while(running) {
        SDL_Event windowEvent;
        while(SDL_PollEvent(&windowEvent))
            if(windowEvent.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }
            draw();
    }
}

void RenderBase::createInstance(){
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

    std::cout << *extensions << std::endl;

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
    vkCreateInstance(&instance_ci, nullptr, &_instance);

    //load function pointers from instance
    volkLoadInstance(_instance);
}

/*
creates:
_physicalDevice
_graphicsQueueIndex
_graphicsQueueCount
_graphicsQueue
_device
 */
void RenderBase::createDevice(){
    //enumerate physical devices
    uint32_t deviceCount = 0;
    
    if(vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr) != VK_SUCCESS){
        std::cout << "could not query physical devices" << std::endl;
        exit(1);
    }

    std::vector<VkPhysicalDevice> queried_devices(deviceCount);

    if(vkEnumeratePhysicalDevices(_instance, &deviceCount, queried_devices.data()) != VK_SUCCESS){
        std::cout << "could not query physical devices" << std::endl;
        exit(1);
    }

    //pick discrete gpu
    VkPhysicalDeviceProperties deviceProperties = {};
    for(auto device : queried_devices){
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            _physicalDevice = device;
        }
    }

    //query for device queues
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    int count = 0;
    for(auto queueFamily : queueFamilyProperties){
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_TRANSFER_BIT != 0){
            _graphicsQueueIndex = count;
            _graphicsQueueCount = queueFamily.queueCount;
        }
        count++;
    }

    //queue create infos
    std::vector<float> priorities(_graphicsQueueCount);
    for(auto priority : priorities){
        priority = 1.0f;
    }
    VkDeviceQueueCreateInfo graphicsQueueCI = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        _graphicsQueueIndex,
        _graphicsQueueCount,
        priorities.data()
    };

    std::vector<VkDeviceQueueCreateInfo> queueCIs(1);
    queueCIs[0] = graphicsQueueCI;

    std::vector<const char*> deviceExtensions = {
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo deviceCI = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           //sType
        nullptr,                                        //pNext
        0,                                              //flags
        1,                                              //queueCreateInfoCount
        queueCIs.data(),                                //pQueueCreateInfos
        0,                                              //enabledLayerCount
        nullptr,                                        //ppEnabledLayerNames
        1,                                              //enabledExtensionCount
        deviceExtensions.data(),                        //ppEnabledExtensionNames
        nullptr                                         //pEnabledFeatures
    };

    //create device
    if(vkCreateDevice(_physicalDevice, &deviceCI, nullptr, &_device) != VK_SUCCESS){
        std::cout << "could not create device" << std::endl;
        exit(1);
    }

    //grab queue handle
    vkGetDeviceQueue(_device, _graphicsQueueIndex, 0, &_graphicsQueue);

}

void RenderBase::getSwapchainImages(){
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, nullptr);
    _swapchainImages.resize(swapchainImageCount);
    if(vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImageCount, _swapchainImages.data()) != VK_SUCCESS){
        std::cout << "could not get swapchain images" << std::endl;
        exit(1);
    }
}

void RenderBase::createImageViews(){
    _swapchainImageViews.resize(_swapchainImages.size());

    for(int i = 0; i < _swapchainImages.size(); i++){

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
            _swapchainImages[i],                                              //image
            VK_IMAGE_VIEW_TYPE_2D,                              //viewType
            VK_FORMAT_R8G8B8A8_SRGB,                            //format
            componentMapping,                                   //components
            imageSubresourceRange                               //subresourceRange
        };

        if(vkCreateImageView(_device, &imageViewCI, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS){
            std::cout << "could not create image view" << std::endl;
            exit(1);
        }
    }
}

void RenderBase::createFramebuffers(){
    _swapchainFramebuffers.resize(_swapchainImageViews.size());

    for(int i = 0; i < _swapchainImageViews.size(); i++){
        VkImageView attachments[] = {
            _swapchainImageViews[i]
        };
        VkFramebufferCreateInfo framebufferCI = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,          //sType
            nullptr,                                            //pNext
            0,                                                  //flags
            _renderPass,                                        //renderPass
            1,                                                  //attachmentCount
            attachments,                                         //pAttachments
            _swapchainExtent.width,                                       //width
            _swapchainExtent.height,                                      //height
            1                                                   //layers
        };

        if(vkCreateFramebuffer(_device, &framebufferCI, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS){
            std::cout << "could not create framebuffer" << std::endl;
            exit(1);
        }
    }
}
/*
creates:
_swapchain
_viewport
*/
void RenderBase::createSwapchain(){
    //query surface capabilities for current extent of surface
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCapabilities);

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount,nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount,presentModes.data());

    for(auto mode : presentModes){
        std::cout << mode << std::endl;
    }

    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &surfaceFormatCount, surfaceFormats.data());

    for(auto format : surfaceFormats){
        std::cout << format.format << std::endl;
    }
    _swapchainExtent = surfaceCapabilities.currentExtent;

    //create viewport based off surface
    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.height = surfaceCapabilities.currentExtent.height;
    _viewport.width = surfaceCapabilities.currentExtent.width;
    _viewport.maxDepth = 1.0f;
    _viewport.minDepth = 0.0f;

    std::vector<uint32_t> queueFamilies = {_graphicsQueueIndex};

    VkSwapchainCreateInfoKHR swapchainCI = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,        //sType
        nullptr,                                            //pNext
        {},                                                  //flags
        _surface,                                           //surface
        2,                                                  //minImageCount
        VK_FORMAT_R8G8B8A8_SRGB,                            //imageFormat
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,                  //imageColorSpace
        surfaceCapabilities.currentExtent,                  //imageExtent
        1,                                                  //imageArrayLayers
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                //imageUsage
        {},                                                 //imageSharingMode
        1,                                                  //queueFamilyIndexCount
        queueFamilies.data(),                               //pQueueFamilyIndices
        surfaceCapabilities.currentTransform,               //preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,                  //compositeAlpha
        VK_PRESENT_MODE_MAILBOX_KHR,                        //presentMode
        VK_FALSE,                                           //clipped
        nullptr                                             //oldSwapchain
    };

    if(vkCreateSwapchainKHR(_device, &swapchainCI, nullptr, &_swapchain) != VK_SUCCESS){
        std::cout << "could not create swapchain" << std::endl;
        exit(1);
    }
}

void RenderBase::recordCommandBuffer(VkCommandBuffer buf, uint32_t imageIndex){
    VkCommandBufferBeginInfo commandBufferBeginCI = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr
    };

    if(vkBeginCommandBuffer(buf, &commandBufferBeginCI)){
        std::cout << "could not start command buffer" << std::endl;
        exit(1);
    }

    //render area
    VkOffset2D offset = {
        0,                                              //x
        0                                               //y
    };

    VkRect2D renderArea = {
        offset,                                         //offset
        {WINDOW_HEIGHT, WINDOW_WIDTH}                                //extent
    };

    //clear color
    VkClearColorValue clearColorValue = {
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkClearValue clearColor = {
        {{0.0f, 0.0f, 0.0f, 0.0f}}
    };

    VkRenderPassBeginInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        _renderPass,
        _swapchainFramebuffers[imageIndex],
        renderArea,
        1,
        &clearColor
    };

    vkCmdBeginRenderPass(buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        vkCmdDraw(buf, 3, 1, 0, 0);
    vkCmdEndRenderPass(buf);

    if(vkEndCommandBuffer(buf) != VK_SUCCESS){
        std::cout << "could not record command buffer" << std::endl;
        exit(1);
    }
}

void RenderBase::createGraphicsPipeline(){
    //SHADER MODULE CREATION
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCIs(2);
    auto vertShaderCode = readFile("C:\\Vulkan\\shaders\\vert.spv");
    auto fragShaderCode = readFile("C:\\Vulkan\\shaders\\frag.spv");

    VkShaderModuleCreateInfo vertModuleCI = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        vertShaderCode.size(),
        reinterpret_cast<const uint32_t*>(vertShaderCode.data())
    };

    VkShaderModuleCreateInfo fragModuleCI = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        fragShaderCode.size(),
        reinterpret_cast<const uint32_t*>(fragShaderCode.data())
    };

    if(vkCreateShaderModule(_device, &vertModuleCI, nullptr, &_vertShader) != VK_SUCCESS){
        std::cout << "could not create vert shader module" << std::endl;
        exit(1);
    }

    if(vkCreateShaderModule(_device, &fragModuleCI, nullptr, &_fragShader) != VK_SUCCESS){
        std::cout << "could not create frag shader module" << std::endl;
        exit(1);
    }

    shaderStageCIs[0] = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    //sType
        nullptr,                                                //pNext
        {},                                                     //flags
        VK_SHADER_STAGE_VERTEX_BIT,                             //stage
        _vertShader,                                            //module
        "main",                                                 //pName
        nullptr                                                 //pSpecializationInfo
    };
    shaderStageCIs[1] = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    //sType
        nullptr,                                                //pNext
        {},                                                     //flags
        VK_SHADER_STAGE_FRAGMENT_BIT,                           //stage
        _fragShader,                                            //module
        "main",                                                 //pName
        nullptr                                                 //pSpecializationInfo
    };

    //VERTEX INPUT STATE
    VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        0,                                                              //vertexBindingDescriptionCount
        nullptr,                                                        //pVertexBindingDescriptions
        0,                                                              //vertexAttributeDescriptionCount
        nullptr                                                         //pVertexAttributeDescriptions
    };

    //INPUT ASSEMBLY STATE
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                            //topology
        VK_FALSE                                                        //primitiveRestartEnable
    };

    //TESSELATION STATE
    VkPipelineTessellationStateCreateInfo tessellationCI = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,      //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        0                                                               //patchControlPoints
    };

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportCI = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,          //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        1,                                                              //viewportCount
        &_viewport,                                               //pViewports
        1,                                                              //scissorCount
        &scissor                                                 //pScissors
    };

    //RASTERIZATION STATE
    VkPipelineRasterizationStateCreateInfo rasterizationCI = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,     //sType
        nullptr,                                                        //pNext
        0,                                                              //flags
        VK_FALSE,                                                       //depthClampEnable
        VK_FALSE,                                                       //rasterizerDiscardEnable
        VK_POLYGON_MODE_LINE,                                           //polygonMode
        VK_CULL_MODE_NONE,                                              //cullMode
        VK_FRONT_FACE_CLOCKWISE,                                        //frontFace
        VK_FALSE,                                                       //depthBiasEnable
        0.0f,                                                           //depthBiasConstantFactor
        0.0f,                                                           //depthBiasClamp
        0.0f,                                                           //depthBiasSlopeFactor
        1.0f                                                            //lineWidth
    };

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

    //COLOR BLEND STATE
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
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
        &colorBlendAttachment,                                          //pAttachments
                                                                        //blendConstants
    };

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

    VkPipelineLayout pipelineLayout = {};
    if(vkCreatePipelineLayout(_device, &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS){
        std::cout << "could not create pipeline layout" << std::endl;
        exit(1);
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCI = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,        //sType 
        nullptr,                                                //pNext
        0,                                                      //flags
        2,                                                      //stageCount
        shaderStageCIs.data(),                                  //pStages
        &vertexInputStateCI,                                    //pVertexInputState
        &inputAssemblyCI,                                       //pInputAssemblyState
        &tessellationCI,                                        //pTessellationState
        &viewportCI,                                            //pViewportState
        &rasterizationCI,                                       //pRasterizationState
        &multisampleCI,                                         //pMultisampleState
        nullptr,                                                //pDepthStencilState
        &colorBlendCI,                                          //pColorBlendState
        nullptr,                                                //pDynamicState
        pipelineLayout,                                         //layout
        _renderPass,                                            //renderPass
        0,                                                      //subpass
        nullptr,                                                //basePipelineHandle
        {}                                                      //basePipelineIndex
    };

    if(vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCI, nullptr, &_pipeline) != VK_SUCCESS){
        std::cout << "could not create pipeline" << std::endl;
        exit(1);
    }
}

void RenderBase::createRenderPass(){
    VkAttachmentDescription colorAttachment = {
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

    VkAttachmentReference colorAttachmentRef = {
        0,                                                      //attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                //layout
    };

    VkSubpassDescription subpassDescription = {
        0,                                                      //flags
        VK_PIPELINE_BIND_POINT_GRAPHICS,                        //pipelineBindPoint
        0,                                                      //inputAttachmentCount
        nullptr,                                                //pInputAttachments
        1,                                                      //colorAttachmentCount
        &colorAttachmentRef,                                    //pColorAttachments
        nullptr,                                                //pResolveAttachments
        nullptr,                                                //pDepthStencilAttachment
        0,                                                      //preserveAttachmentCount
        nullptr                                                 //pPreserveAttachments

    };

    VkSubpassDependency dependency = {
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
        &colorAttachment,                                       //pAttachments
        1,                                                      //subpassCount
        &subpassDescription,                                    //pSubpasses
        1,                                                      //dependencyCount
        &dependency                                             //pDependencies
    };
    
    if(vkCreateRenderPass(_device, &renderPassCI, nullptr, &_renderPass) != VK_SUCCESS){
        std::cout << "could not create render pass" << std::endl;
    }
}

void RenderBase::createWindow(){
    _window = SDL_CreateWindow("hello", WINDOW_HEIGHT, WINDOW_WIDTH, SDL_WINDOW_VULKAN);
    if(false == SDL_Vulkan_CreateSurface(_window, _instance, nullptr, &_surface)){
        std::cout << SDL_GetError() << std::endl;
    }
    //SDL_SetWindowFullscreen(_window, true);
    //SDL_GetWindowSize(_window, reinterpret_cast<int*>(&WINDOW_WIDTH), reinterpret_cast<int*>(&WINDOW_HEIGHT));
}

void RenderBase::draw(){
    uint32_t imageIndex = 0;

    vkWaitForFences(_device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device, 1, &_inFlightFence);

    vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _imageAvailableSemaphore, nullptr, &imageIndex);
    vkResetCommandBuffer(_commandBuffer, 0);
    recordCommandBuffer(_commandBuffer, imageIndex);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                      //sType
        nullptr,                                            //pNext
        1,                                                  //waitSemaphoreCount
        &_imageAvailableSemaphore,                          //pWaitSemaphores
        waitStages,                                         //pWaitDstStageMask
        1,                                                  //commandBufferCount
        &_commandBuffer,                                    //pCommandBuffers
        1,                                                  //signalSemaphoreCount
        &_renderFinishedSemaphore                           //pSignalSemaphores
    };

    if(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFence) != VK_SUCCESS){
        std::cout << "could not submit command buffer to queue" << std::endl;
        exit(1);
    }

    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &_renderFinishedSemaphore,
        1,
        &_swapchain,
        &imageIndex,
    };

    std::cout << vkQueuePresentKHR(_graphicsQueue, &presentInfo);
}

void RenderBase::createSyncObjects(){
    VkSemaphoreCreateInfo semaphoreInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,            //sType
        nullptr                                             //pNext
    };

    VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,                //sType
        nullptr,                                            //pNext
        VK_FENCE_CREATE_SIGNALED_BIT                        //flags
    };

    if(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore) ||
       vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore) ||
       vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFence) != VK_SUCCESS){
        std::cout << "could not create sync objects" << std::endl;
        exit(1);     
    }
}

void RenderBase::createCommandBuffer(){
    VkCommandPoolCreateInfo commandPoolCI = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        _graphicsQueueIndex
    };

    if(vkCreateCommandPool(_device, &commandPoolCI, nullptr, &_commandPool) != VK_SUCCESS){
        std::cout << "could not create command pool" << std::endl;
        exit(1);
    }

    VkCommandBufferAllocateInfo commandBufferAllocateI = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        _commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    if(vkAllocateCommandBuffers(_device, &commandBufferAllocateI, &_commandBuffer) != VK_SUCCESS){
        std::cout << "could not create command buffer" << std::endl;
        exit(1);
    }

}
