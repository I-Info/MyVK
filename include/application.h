#ifndef MYVK_APPLICATION_H
#define MYVK_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class Application {
public:
  void run();

private:
  const uint32_t WIDTH = 800;
  const uint32_t HEIGHT = 600;

  static const int DEVICE_EXTENSIONS_COUNT = 2;
  static const char *DEVICE_EXTENSIONS[];

#ifndef NDEBUG
  VkDebugReportCallbackEXT callback{};
#endif

  GLFWwindow *window{};

  VkInstance instance{};

  VkPhysicalDevice physicalDevice{};
  VkDevice device{};

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;

  VkQueue graphicsQueue{};
  VkQueue presentQueue{};

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;

  VkSurfaceKHR surface{};

  struct QueueFamilyIndices {
    static const uint32_t GRAPHICS; // 0b01
    static const uint32_t PRESENT;  // 0b10
    static const int FLAGS = 2;
    uint32_t indices[FLAGS]{};
    uint32_t flag = 0B00;

    inline bool isComplete() const { return flag == 0B11; }
    inline bool checkFlag(const uint32_t &f) const { return (flag & f) == 0; }
    static inline uint32_t flag2BitIndex(const uint32_t &f);
    void setIndex(const uint32_t &f, const uint32_t &value);
    uint32_t getIndex(const uint32_t &i, bool byBitIndex = false) const;
  };

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  void initWindow();

  void initVulkan();

  void createInstance();

// for debug
#ifndef NDEBUG

  void setUpDebugCallback();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                uint64_t obj, size_t location, int32_t code,
                const char *layerPrefix, const char *msg, void *userData);

  static VkResult CreateDebugReportCallbackEXT(
      VkInstance instance,
      const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugReportCallbackEXT *pCallback);
  static void
  DestroyDebugReportCallbackEXT(VkInstance instance,
                                VkDebugReportCallbackEXT callback,
                                const VkAllocationCallbacks *pAllocator);

#endif

  void createSurface();

  void selectPhysicalDevices(QueueFamilyIndices &indices,SwapChainSupportDetails &swapChainSupport);

  bool isDeviceSuitable(const VkPhysicalDevice &dev,
                        QueueFamilyIndices &indices,
                        SwapChainSupportDetails &swapChainSupport);

  static bool checkDeviceExtensions(const VkPhysicalDevice &dev);

  SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &dev);

  bool findQueueFamilies(const VkPhysicalDevice &dev,
                         Application::QueueFamilyIndices &indices);

  void createLogicalDevice(const QueueFamilyIndices &queueFamilyIndices);

  static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  static VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  void createSwapChain(const SwapChainSupportDetails &swapChainSupport,
                       const QueueFamilyIndices &indices);

  void createImageViews();

  void createRenderPass();

  void createGraphicsPipeline();

  VkShaderModule createShaderModule(const std::vector<char>& code);

  void mainLoop();

  void cleanUp();
};

#endif