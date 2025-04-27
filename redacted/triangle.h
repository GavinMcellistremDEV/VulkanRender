class RenderBase{
    private:
        int WINDOW_HEIGHT = 900;
        int WINDOW_WIDTH = 900;

        VkSemaphore _imageAvailableSemaphore;
        VkSemaphore _renderFinishedSemaphore;
        VkFence _inFlightFence;

        SDL_Window* _window;
        VkSurfaceKHR _surface;

        VkRenderPass _renderPass;

        VkPipeline _pipeline;

        VkExtent2D _swapchainExtent;
        VkSwapchainKHR _swapchain;

        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        std::vector<VkFramebuffer> _swapchainFramebuffers;

        VkViewport _viewport;

        VkInstance _instance;

        VkPhysicalDevice _physicalDevice;
        VkDevice _device;

        VkQueue _graphicsQueue;
        uint32_t _graphicsQueueIndex;
        uint32_t _graphicsQueueCount;

        VkCommandPool _commandPool;
        VkCommandBuffer _commandBuffer;

        VkShaderModule _vertShader;
        VkShaderModule _fragShader;


    public:
        RenderBase();

        void init();
        void mainLoop();
        void createRenderPass();
        void createInstance();
        void createDevice();
        void getSwapchainImages();
        void createImageViews();
        void createFramebuffers();
        void createCommandBuffer();
        void createSwapchain();
        void recordCommandBuffer(VkCommandBuffer, uint32_t);
        void createWindow();
        void createGraphicsPipeline();
        void draw();
        void createSyncObjects();
};
