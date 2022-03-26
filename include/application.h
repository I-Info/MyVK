#ifndef MYVK_APPLICATION_H
#define MYVK_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

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

  void initVulkan();

  void createInstance();

// for debug
#ifndef NDEBUG
  void setUpDebugCallback();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                uint64_t obj, size_t location, int32_t code,
                const char *layerPrefix, const char *msg, void *userData);

  VkResult CreateDebugReportCallbackEXT(
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

  void selectPhysicalDevices();

  bool isDeviceSuitable(const VkPhysicalDevice &device);

  bool findQueueFamily(const VkPhysicalDevice &device);

  void mainLoop();

  void cleanUp();
};

#endif