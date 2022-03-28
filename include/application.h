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
  VkDebugReportCallbackEXT callback;
#endif

  GLFWwindow *window;

  VkInstance instance;

  VkDevice device;

  VkQueue graphicsQueue;

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

  void selectPhysicalDevices(VkPhysicalDevice &physicalDevice,
                             uint32_t &queueFamilyIndex);

  static bool isDeviceSuitable(const VkPhysicalDevice &dev, uint32_t &index);

  static bool findQueueFamily(const VkPhysicalDevice &dev, uint32_t &index);

  void createLogicalDevice(const VkPhysicalDevice &physicalDevice,
                           const uint32_t &queueFamilyIndex);

  void mainLoop();

  void cleanUp();
};

#endif