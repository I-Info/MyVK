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
  VkPhysicalDevice physicalDevice;
  uint32_t index;
  selectPhysicalDevices(physicalDevice, index);
  createLogicalDevice(physicalDevice, index);
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

  uint32_t extensionCount = 0;

  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&extensionCount);

#ifndef NDEBUG
  const char **extensions = new const char *[extensionCount + 1];
  for (int i = 0; i < extensionCount; ++i) {
    size_t len = std::strlen(glfwExtensions[i]);
    char *tmp = new char[len];
    std::memcpy(tmp, glfwExtensions[i], sizeof(char) * len);
    extensions[i] = tmp;
  }

  delete[] glfwExtensions;

  extensions[extensionCount] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
  createInfo.enabledExtensionCount = extensionCount + 1;
  createInfo.ppEnabledExtensionNames = extensions;
#else

  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
#endif
  createInfo.enabledLayerCount = 0;

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
  LOG(ERROR) << "validation layer: " << msg << std::endl;
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

void Application::selectPhysicalDevices(VkPhysicalDevice &physicalDevice,
                                        uint32_t &queueFamilyIndex) {
  physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("No available physical devices found.");
  }
  auto *devices = new VkPhysicalDevice[deviceCount];
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  // find first suitable physical device
  for (VkPhysicalDevice *d = devices; d != devices + deviceCount; ++d) {
    if (isDeviceSuitable(*d, queueFamilyIndex)) {
      physicalDevice = *d;
      break;
    }
  }
  delete[] devices;

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Failed to find a suitable GPU.");
  }
}

bool Application::isDeviceSuitable(const VkPhysicalDevice &dev,
                                   uint32_t &index) {
  // check dev suitability
#ifndef NDEBUG
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(dev, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

  LOG(INFO) << "Find physical dev: " << deviceProperties.deviceID << " "
            << deviceProperties.vendorID << " " << deviceProperties.deviceName;
#endif
  return findQueueFamily(dev, index);
}

bool Application::findQueueFamily(const VkPhysicalDevice &dev,
                                  uint32_t &index) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);
  auto *queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount,
                                           queueFamilies);
  uint32_t i;
  for (i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueCount > 0 &&
        queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      break;
    }
  }
  delete[] queueFamilies;
  index = i;
  return i != queueFamilyCount;
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
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Application::createLogicalDevice(const VkPhysicalDevice &physicalDevice,
                                      const uint32_t &queueFamilyIndex) {
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = new float{1.0f};

  VkPhysicalDeviceFeatures deviceFeatures = {};

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = &queueCreateInfo;
  createInfo.queueCreateInfoCount = 1;
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = 0;
  createInfo.enabledLayerCount = 0;

  if (vkCreateDevice(physicalDevice,&createInfo, nullptr,&device) != VK_SUCCESS) {
    throw std::runtime_error("Fail to create logical device!");
  };

  vkGetDeviceQueue(device,queueFamilyIndex,0,&graphicsQueue);
}
