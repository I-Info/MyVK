#include "application.h"
#include "logging.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <cstring>
#include <functional>
#include <iostream>

const uint32_t Application::QueueFamilyIndices::GRAPHICS = 0B01;
const uint32_t Application::QueueFamilyIndices::PRESENT = 0B10;

void Application::run() {
  initWindow();
  initVulkan();
  mainLoop();
  cleanUp();
}

void Application::initVulkan() {
  createInstance();
#ifndef NDEBUG
  setUpDebugCallback();
#endif
  createSurface();
  VkPhysicalDevice physicalDevice;
  QueueFamilyIndices indices;
  selectPhysicalDevices(physicalDevice, indices);
  createLogicalDevice(physicalDevice, indices);
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

  if (glfwVulkanSupported() != GLFW_TRUE) {
    LOG(ERROR) << "No Vulkan support!";
  }

  uint32_t extensionCount = 0;

  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&extensionCount);
  if (glfwExtensions == nullptr) {
    LOG(ERROR) << "Fail to get vulkan extensions.";
  }

#ifndef NDEBUG
  const char **extensions = new const char *[extensionCount + 1];
  std::memcpy(extensions, glfwExtensions, sizeof(char *) * extensionCount);

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
    LOG(ERROR) << "Fail to create Vulkan instance";
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
    LOG(ERROR) << "failed to set up debug callback!";
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
  // disable API
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Hello", nullptr, nullptr);
}

void Application::selectPhysicalDevices(VkPhysicalDevice &physicalDevice,
                                        QueueFamilyIndices &indices) {
  physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    LOG(ERROR) << "No available physical devices found.";
  }
  auto *devices = new VkPhysicalDevice[deviceCount];
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  // find first suitable physical device
  for (VkPhysicalDevice *d = devices; d != devices + deviceCount; ++d) {
    if (isDeviceSuitable(*d, indices)) {
      physicalDevice = *d;
      break;
    }
  }
  delete[] devices;

  if (physicalDevice == VK_NULL_HANDLE) {
    LOG(ERROR) << "Failed to find a suitable GPU.";
  }
}

bool Application::isDeviceSuitable(const VkPhysicalDevice &dev,
                                   QueueFamilyIndices &indices) {
  // check dev suitability
#ifndef NDEBUG
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(dev, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

  LOG(INFO) << "Find physical dev: " << deviceProperties.deviceID << " "
            << deviceProperties.vendorID << " " << deviceProperties.deviceName;
#endif
  findQueueFamilies(dev, indices);
  return indices.isComplete();
}

void Application::findQueueFamilies(const VkPhysicalDevice &dev,
                                    QueueFamilyIndices &indices) {
  // get properties
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);
  auto *queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount,
                                           queueFamilies);
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (indices.checkFlag(QueueFamilyIndices::GRAPHICS) &&
        queueFamilies[i].queueCount > 0 &&
        queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.setIndex(QueueFamilyIndices::GRAPHICS, i);
    }

    if (indices.checkFlag(QueueFamilyIndices::PRESENT)) {
      VkBool32 presentSupport = false;
      if (queueFamilies[i].queueCount > 0 &&
          vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface,
                                               &presentSupport) == VK_SUCCESS &&
          presentSupport) {
        indices.setIndex(QueueFamilyIndices::PRESENT, i);
      }
    }

    if (indices.isComplete()) {
      break;
    }
  }

  delete[] queueFamilies;
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
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Application::createLogicalDevice(
    const VkPhysicalDevice &physicalDevice,
    const QueueFamilyIndices &queueFamilyIndices) {

  auto *queueCreateInfos =
      new VkDeviceQueueCreateInfo[QueueFamilyIndices::FLAGS];

  for (int i = 0; i < QueueFamilyIndices::FLAGS; ++i) {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex = queueFamilyIndices.getIndex(i, true);
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].pQueuePriorities = new float{1.0f};
  }

  auto *deviceFeatures = new VkPhysicalDeviceFeatures{};

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.queueCreateInfoCount = QueueFamilyIndices::FLAGS;
  createInfo.pEnabledFeatures = deviceFeatures;
  createInfo.enabledExtensionCount = 0;
  createInfo.enabledLayerCount = 0;

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
      VK_SUCCESS) {
    LOG(ERROR) << "Fail to create logical device!";
  }

  vkGetDeviceQueue(device,
                   queueFamilyIndices.getIndex(QueueFamilyIndices::GRAPHICS), 0,
                   &graphicsQueue);
  vkGetDeviceQueue(device,
                   queueFamilyIndices.getIndex(QueueFamilyIndices::PRESENT), 0,
                   &presentQueue);
}

void Application::createSurface() {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
      VK_SUCCESS) {
    LOG(ERROR) << "Fail to create window surface";
  }
}

void Application::QueueFamilyIndices::setIndex(const uint32_t &f,
                                               const uint32_t &value) {
  this->indices[flag2BitIndex(f)] = value;
  flag |= f;
}
uint32_t Application::QueueFamilyIndices::getIndex(const uint32_t &i,
                                                   bool byBit) const {
  if (byBit) {
    return indices[i];
  }
  return indices[flag2BitIndex(i)];
}

inline uint32_t
Application::QueueFamilyIndices::flag2BitIndex(const uint32_t &f) {
  uint32_t bit, t;
  for (bit = 0, t = f; t > 1; t >>= 1, ++bit)
    ;
  return bit;
}
