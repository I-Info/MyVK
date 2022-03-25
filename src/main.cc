#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
              uint64_t obj, size_t location, int32_t code,
              const char *layerPrefix, const char *msg, void *userData) {

  std::cerr << "validation layer: " << msg << std::endl;

  return VK_FALSE;
}

#endif
class MyApplication {
public:
  void run() {
    initVulkan();
    initWindow();
    mainLoop();
    cleanUp();
  }

private:
  const int WIDTH = 800;
  const int HEIGHT = 600;

#ifndef NDEBUG
  VkDebugReportCallbackEXT callback;
#endif

  GLFWwindow *window;

  VkInstance instance;

  void initVulkan() {
    createInstance();
#ifndef NDEBUG
    setUpDebugCallback();
#endif
  }

  void createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Hello World";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NO Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (glfwVulkanSupported() == GLFW_TRUE) {
      throw std::runtime_error("No Vulkan support!");
    }

    uint32_t extentionCount = 0;

    const char **glfwExtentions =
        glfwGetRequiredInstanceExtensions(&extentionCount);

    // check & enable validation layer
#ifndef NDEBUG
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    VkLayerProperties *availableProperties = new VkLayerProperties[layerCount];
    char **layerNames = new char *[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableProperties);
    std::cout << "Vulkan available layer properties: " << std::endl;

    bool flag = false;
    for (uint32_t i = 0; i < layerCount; ++i) {
      std::cout << "\t" << (availableProperties[i]).layerName << std::endl;
      layerNames[i] = new char[256];
      std::memcpy(layerNames[i], (availableProperties[i]).layerName, 256);
      if (std::strcmp("VK_LAYER_KHRONOS_validation",
                      (availableProperties[i]).layerName))
        flag = true;
    }
    delete[] availableProperties;
    if (!flag) {
      delete[] layerNames;
      throw std::runtime_error("[error] Validation layer not found.");
    }
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = layerNames;

    const char **extentions = new const char *[extentionCount + 1];
    for (int i = 0; i < extentionCount; ++i) {
      char *tmp;
      std::strcpy(tmp, glfwExtentions[i]);
      extentions[i] = tmp;
    }

    delete[] glfwExtentions;

    extentions[extentionCount] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    createInfo.enabledExtensionCount = extentionCount + 1;
    createInfo.ppEnabledExtensionNames = extentions;

#else
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = extentionCount;
    createInfo.ppEnabledExtensionNames = glfwExtentions;
#endif

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Fail to create Vulkan instance");
    }
  }

// for debug
#ifndef NDEBUG
  void setUpDebugCallback() {
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr,
                                     &callback) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug callback!");
    }
  }

  VkResult CreateDebugReportCallbackEXT(
      VkInstance instance,
      const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugReportCallbackEXT *pCallback) {
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }
  static void
  DestroyDebugReportCallbackEXT(VkInstance instance,
                                VkDebugReportCallbackEXT callback,
                                const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
      func(instance, callback, pAllocator);
    }
  }
#endif

  void initWindow() {
    glfwInit();
    // disable OpenGL API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello", nullptr, nullptr);
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  void cleanUp() {
#ifndef NDEBUG
    DestroyDebugReportCallbackEXT(instance, callback, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
  }
};

int main() {
  MyApplication application;
  try {
    application.run();
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
