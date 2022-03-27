#include "application.h"
#include "logging.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>

void Application::run() {
  initVulkan();
  initWindow();
  mainLoop();
  cleanUp();
}

void Application::initVulkan() {
  createInstance();
#ifndef NDEBUG
  setUpDebugCallback();
#endif
  selectPhysicalDevices();
}

void Application::createInstance() {
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

#ifndef NDEBUG
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

#ifndef NDEBUG
void Application::setUpDebugCallback() {
  VkDebugReportCallbackCreateInfoEXT createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags =
      VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  createInfo.pfnCallback = Application::debugCallback;

  if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
    uint64_t obj, size_t location, int32_t code, const char *layerPrefix,
    const char *msg, void *userData) {

  std::cerr << "validation layer: " << msg << std::endl;

  return VK_FALSE;
}

VkResult Application::CreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
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

void Application::DestroyDebugReportCallbackEXT(
    VkInstance instance, VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}
#endif

void Application::initWindow() {
  glfwInit();
  // disable OpenGL API
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Hello", nullptr, nullptr);
}

void Application::selectPhysicalDevices() {
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("No available physical devices found.");
  }
  VkPhysicalDevice *devices = new VkPhysicalDevice[deviceCount];
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  // find first suitable physical device
  for (VkPhysicalDevice *device = devices; device != devices + deviceCount;
       ++device) {
    if (isDeviceSuitable(*device)) {
      physicalDevice = *device;
      break;
    }
  }
  delete[] devices;

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Failed to find a suitable GPU.");
  }
}

bool Application::isDeviceSuitable(const VkPhysicalDevice &device) {
  // check device suitability
#ifndef NDEBUG
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  LOG(INFO) << "Find physical device: " << deviceProperties.deviceID << " "
            << deviceProperties.vendorID << " " << deviceProperties.deviceName;
#endif
  return findQueueFamily(device);
}

bool Application::findQueueFamily(const VkPhysicalDevice &device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  VkQueueFamilyProperties *queueFamilies =
      new VkQueueFamilyProperties[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies);
  bool flag = false;
  for (auto queueFamily = queueFamilies;
       queueFamily != queueFamilies + queueFamilyCount; queueFamily++) {
    if (queueFamily->queueCount > 0 &&
        queueFamily->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      flag = true;
      break;
    }
  }
  delete[] queueFamilies;
  return flag;
}

void Application::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void Application::cleanUp() {
#ifndef NDEBUG
  DestroyDebugReportCallbackEXT(instance, callback, nullptr);
#endif
  vkDestroyInstance(instance, nullptr);
  glfwDestroyWindow(window);
  glfwTerminate();
}