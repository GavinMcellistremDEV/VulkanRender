#include "RenderBase.h" 
#include "iostream"

#define WIDTH 1000
#define HEIGHT 1000
float clearColor[4] = {0.0f,0.0f,0.0f,0.0f};

int main(){
    std::vector<Vertex> vertices = { 
        {{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, 
        {{-0.25f, 0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.25f, 0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.5f, 0.25f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.5f, -0.25f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.25f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{-0.25f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{-0.5f, -0.25f}, {1.0f, 0.0f, 0.0f}}, 
        {{-0.5f, 0.25f}, {1.0f, 0.0f, 0.0f}}, 
    };
    //012023034045056067078081

    const std::vector<uint16_t> indices = {
        0,1,2,0,2,3,0,3,4,0,4,5,0,5,6,0,6,7,0,7,8,0,8,1
    };
    
    Context context;
    context.initContext();

    Display display;
    display.initDisplay(context, WIDTH, HEIGHT);

    CommandBuffer commandBuffer;
    commandBuffer.initCommandBuffer(context);

    std::vector<Image> images;
    images = display.getImagesAndViews(context);

    RenderPass renderPass;
    renderPass.setRenderArea(WIDTH, HEIGHT);
    renderPass.setClearColor(clearColor);
    renderPass.initRenderPass(context, commandBuffer);
    renderPass.createFramebuffers(context, images, display.swapchainExtent);

    PipelineBuilder pipelineBuilder;
    pipelineBuilder.setShader(context, VK_SHADER_STAGE_VERTEX_BIT, "C:\\Vulkan\\shaders\\vert.spv", "main");
    pipelineBuilder.setShader(context, VK_SHADER_STAGE_FRAGMENT_BIT, "C:\\Vulkan\\shaders\\frag.spv", "main");
    pipelineBuilder.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.setVertexInputState();
    pipelineBuilder.setTessellationState();
    pipelineBuilder.setViewportState(display.viewport, display.defaultScissor);
    pipelineBuilder.setRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, 1.0f);
    pipelineBuilder.setMultisampleState();
    pipelineBuilder.setColorblendState();
    pipelineBuilder.setPipelineLayout(context);

    VkPipeline graphicsPipeline = pipelineBuilder.createPipeline(context, renderPass);

    Semaphore imageAvailableSem;
    imageAvailableSem.initSemaphore(context);

    Semaphore renderFinishedSem;
    renderFinishedSem.initSemaphore(context);

    Fence inFlight;
    inFlight.initFence(context, true);

    VertexBuffer vBuffer;
    vBuffer.init(context, vertices);

    IndexBuffer iBuffer;
    iBuffer.init(context, indices);
    
    bool running = true;
    uint32_t imageIndex = 0;

    while(running) {
        SDL_Event windowEvent;
        while(SDL_PollEvent(&windowEvent))
            if(windowEvent.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }
            inFlight.wait(context);
            inFlight.reset(context);
            imageIndex = display.getNextPresentableSwapchainIndex(context, display, imageAvailableSem);
            renderPass.startRenderPass(graphicsPipeline, imageIndex);
                vBuffer.bind(commandBuffer);
                iBuffer.bind(commandBuffer);
                renderPass.drawIndexed(indices.size());
            renderPass.endRenderPass();

            renderPass.submitWork(context, imageAvailableSem, renderFinishedSem, inFlight);
            renderPass.submitPresentation(context, display, renderFinishedSem, imageIndex);
    }
    
    return 0;
}