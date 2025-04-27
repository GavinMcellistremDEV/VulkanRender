#pragma once
//VOLK
#include "volk/volk.h"

//SDL
#include "SDL.h"
#include "SDL_vulkan.h"

//GLM
#include "glm/glm.hpp"

//std
#include "string"
#include "vector"
#include "Array"

class Vertex{
    private:
    public:
        glm::vec2 pos;
        glm::vec3 color;
        Vertex(glm::vec2, glm::vec3);
        static VkVertexInputBindingDescription * getBindingDescription();

        static std::vector<VkVertexInputAttributeDescription> * getAttributeDescriptions();
};

struct DeviceQueue{
    VkQueue queueFamily;
    uint32_t queueFamilyIndex;
    uint32_t queueCount;
};

class Context{
    private:
        void createInstance();
        void createPhysicalDevice();
        void createLogicalDeviceAndQueue();

    public:
        VkInstance instance;

        VkPhysicalDevice physicalDevice;
        VkDevice device;
        DeviceQueue queue;

        void initContext();
};

class Image{
    public:
        VkImage image;
        VkImageView imageView;

        void createImageView(Context &);

};

class CommandBuffer{

    void createPool(Context &);
    void allocateBuffer(Context &, VkCommandPool &);

    public:
        VkCommandPool pool;
        VkCommandBuffer buffer;

        void initCommandBuffer(Context &);
        void initCommandBuffer(Context &, VkCommandPool &);
};

class Fence{
    public:
        VkFence fence;
        void initFence(Context &, bool);
        void wait(Context &);
        void reset(Context &);
};

class Semaphore{
    public:
        VkSemaphore semaphore;
        void initSemaphore(Context &);
};

class Display{
    private:
        void createWindowAndSurface(Context &, int, int);
        void createSwapchain(Context &); 

    public:
        SDL_Window* window;
        VkSurfaceKHR surface;
        
        VkViewport viewport;
        VkRect2D defaultScissor;

        VkSwapchainKHR swapchain;
        VkExtent2D swapchainExtent;

        void initDisplay(Context &, int, int);
        std::vector<Image> getImagesAndViews(Context &);
        uint32_t getNextPresentableSwapchainIndex(Context &, Display &, Semaphore &);
};

class RenderPass{
    public:
        CommandBuffer commandBuffer;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> frameBuffers;
        VkRect2D renderArea;
        VkClearValue clearColor;

        void initRenderPass(Context &, CommandBuffer &);
        void createFramebuffers(Context &, std::vector<Image>, VkExtent2D &);
        void setRenderArea(int, int);
        void setClearColor(float[4]);
        void startRenderPass(VkPipeline &, int);
        void drawVertices(VkPipeline, int);
        void drawIndexed(int);
        void endRenderPass();
        void submitWork(Context &, Semaphore &, Semaphore &, Fence &);
        void submitPresentation(Context &, Display &, Semaphore &, uint32_t);
};

class PipelineBuilder{
    private:
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
        VkPipelineVertexInputStateCreateInfo vertexInputState;
        VkPipelineTessellationStateCreateInfo tessellationState;
        VkPipelineViewportStateCreateInfo viewportState;
        VkPipelineRasterizationStateCreateInfo rasterizationState;
        VkPipelineMultisampleStateCreateInfo multisampleState;
        VkPipelineColorBlendStateCreateInfo colorblendState;

    public:
        PipelineBuilder() = default;
        void setShader(Context &, VkShaderStageFlagBits, std::string, std::string); 
        void setInputAssembly(VkPrimitiveTopology);
        void setVertexInputState();
        void setTessellationState();
        void setViewportState(VkViewport &, VkRect2D &);
        void setRasterizationState(VkPolygonMode, VkCullModeFlagBits, VkFrontFace, float);
        void setMultisampleState();
        void setColorblendState();
        void setPipelineLayout(Context &);
        VkPipeline & createPipeline(Context &, RenderPass &);
};

class Buffer{
    private:
        void * mappedMemory;
        VkDeviceSize bufferSize;
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
        uint32_t findMemoryType(Context & context, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    public:
        Buffer() = default;
        void init(Context &, VkDeviceSize size, VkBufferUsageFlagBits bufType, VkSharingMode sharingMode);
        void map(Context &, void * data);
        VkBuffer getBuffer(){return buffer;}
};

class VertexBuffer{
    private:
        Buffer buffer;
    public:
        VertexBuffer() = default;
        void init(Context &, std::vector<Vertex> vertices);
        void bind(CommandBuffer &);
};

class IndexBuffer{
    private:
        Buffer buffer;
    public:
        IndexBuffer() = default;
        void init(Context &, std::vector<uint16_t> indices);
        void bind(CommandBuffer &);
        
};



