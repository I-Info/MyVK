#ifndef MYVK_APPLICATION_H
#define MYVK_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class Application {
public:
  void run();

private:
  const int WIDTH = 800;
  const int HEIGHT = 600;

#ifndef NDEBUG
  VkDebugReportCallbackEXT callback{};
#endif

  GLFWwindow *window{};

  VkInstance instance{};

  VkDevice device{};

  VkQueue graphicsQueue{};
  VkQueue presentQueue{};

  VkSurfaceKHR surface{};

  struct QueueFamilyIndices {
    static const uint32_t GRAPHICS; // 0b01
    static const uint32_t PRESENT;  // 0b10
    static const uint32_t FLAGS = 2;
    uint32_t indices[FLAGS]{};
    uint32_t flag = 0B00;

    inline bool isComplete() const { return flag == 0B11; }
    inline bool checkFlag(const uint32_t &f) const { return (flag & f) == 0; }
    void setIndex(const uint32_t &f, const uint32_t &value);
    uint32_t getIndex(const uint32_t &i, bool byBit = false) const;
    static inline uint32_t flag2BitIndex(const uint32_t &f);
  };

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

  void initWindow();

  void createSurface();

  void selectPhysicalDevices(VkPhysicalDevice &physicalDevice,
                             QueueFamilyIndices &indices);

  bool isDeviceSuitable(const VkPhysicalDevice &dev,
                        QueueFamilyIndices &indices);

  void findQueueFamilies(const VkPhysicalDevice &dev,
                         Application::QueueFamilyIndices &indices);

  void createLogicalDevice(const VkPhysicalDevice &physicalDevice,
                           const QueueFamilyIndices &queueFamilyIndices);

  void mainLoop();

  void cleanUp();
};

#endif