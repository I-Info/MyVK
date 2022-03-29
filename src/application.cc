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

const char *Application::DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};

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
  QueueFamilyIndices indices;
  SwapChainSupportDetails swapChainSupportDetails;
  selectPhysicalDevices(indices, swapChainSupportDetails);
  createLogicalDevice(indices);
  createSwapChain(swapChainSupportDetails, indices);
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

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  if (glfwExtensions == nullptr) {
    LOG(ERROR) << "Fail to get vulkan extensions.";
  }

#ifndef NDEBUG
  uint32_t extensionCount = glfwExtensionCount + 2;
  const char **extensions = new const char *[extensionCount];
  std::memcpy(extensions, glfwExtensions, sizeof(char *) * glfwExtensionCount);
  extensions[extensionCount - 2] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
  extensions[extensionCount - 1] =
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#else
  uint32_t extensionCount = glfwExtensionCount + 1;
  const char **extensions = new const char *[extensionCount];
  std::memcpy(extensions, glfwExtensions, sizeof(char *) * glfwExtensionCount);
  extensions[extensionCount - 1] =
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#endif
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
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
  LOG(WARNING) << msg;
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

  window = glfwCreateWindow(static_cast<int>(WIDTH), static_cast<int>(HEIGHT),
                            "Hello", nullptr, nullptr);
}

void Application::selectPhysicalDevices(
    QueueFamilyIndices &indices, SwapChainSupportDetails &swapChainSupport) {
  physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    LOG(ERROR) << "No available physical devices found.";
  }
  auto *devices = new VkPhysicalDevice[deviceCount];
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  // find first suitable physical device
  for (auto *d = devices; d != devices + deviceCount; ++d) {
    if (isDeviceSuitable(*d, indices, swapChainSupport)) {
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
                                   QueueFamilyIndices &indices,
                                   SwapChainSupportDetails &swapChainSupport) {
  // check dev suitability
#ifndef NDEBUG
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(dev, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

  LOG(INFO) << "Find physical dev: " << deviceProperties.deviceID << " "
            << deviceProperties.vendorID << " " << deviceProperties.deviceName;
#endif
  if (checkDeviceExtensions(dev)) {
    swapChainSupport = querySwapChainSupport(dev);
    if (swapChainSupport.formats.empty() ||
        swapChainSupport.presentModes.empty()) {
      return false;
    }
    return findQueueFamilies(dev, indices);
  }
  return false;
}

bool Application::checkDeviceExtensions(VkPhysicalDevice const &dev) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);
  auto *availableExtensions = new VkExtensionProperties[extensionCount];
  vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount,
                                       availableExtensions);
  int flag = 0;
  for (auto extension = availableExtensions;
       extension != availableExtensions + extensionCount; ++extension) {
    for (auto e : DEVICE_EXTENSIONS) {
      if (std::strcmp(e, extension->extensionName) == 0)
        ++flag;
    }
  }
  delete[] availableExtensions;
  return flag == DEVICE_EXTENSIONS_COUNT;
}

bool Application::findQueueFamilies(const VkPhysicalDevice &dev,
                                    QueueFamilyIndices &indices) {
  // get properties
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);
  if (queueFamilyCount == 0) {
    return false;
  }
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
      delete[] queueFamilies;
      return true;
    }
  }

  delete[] queueFamilies;
  return false;
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
  vkDestroySwapchainKHR(device, swapChain, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Application::createLogicalDevice(
    const QueueFamilyIndices &queueFamilyIndices) {
  VkDeviceQueueCreateInfo queueCreateInfos[QueueFamilyIndices::FLAGS]{};
  float priority = 1.0f;
  for (int i = 0; i < QueueFamilyIndices::FLAGS; ++i) {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex = queueFamilyIndices.getIndex(i, true);
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].pQueuePriorities = &priority;
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  if (queueFamilyIndices.getIndex(QueueFamilyIndices::GRAPHICS) !=
      queueFamilyIndices.getIndex(QueueFamilyIndices::PRESENT)) {
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = QueueFamilyIndices::FLAGS;
  } else {
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = 1;
  }

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = DEVICE_EXTENSIONS_COUNT;
  createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
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

Application::SwapChainSupportDetails
Application::querySwapChainSupport(VkPhysicalDevice const &dev) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount,
                                         details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount,
                                            nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount,
                                              details.presentModes.data());
  }

  return details;
}
VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  if (availableFormats.size() == 1 &&
      availableFormats[0].format == VK_FORMAT_UNDEFINED) {
    return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  }

  for (const auto &format : availableFormats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &presentMode : availablePresentModes) {
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return presentMode;
    if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
      return presentMode;
  }
  return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D
Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    // auto match window extent
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = {WIDTH, HEIGHT};

    // min <= actual extent <= max
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}
void Application::createSwapChain(
    const SwapChainSupportDetails &swapChainSupport,
    const QueueFamilyIndices &indices) {
  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  // maxImageCount = 0 stand for no limit, min <= image count <= max
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1; // 2D
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (indices.getIndex(QueueFamilyIndices::GRAPHICS) !=
      indices.getIndex(QueueFamilyIndices::PRESENT)) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = indices.indices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS) {
    LOG(ERROR) << "Fail to create swap chain.";
  }
}

void Application::QueueFamilyIndices::setIndex(const uint32_t &f,
                                               const uint32_t &value) {
  this->indices[flag2BitIndex(f)] = value;
  flag |= f;
}

uint32_t Application::QueueFamilyIndices::getIndex(const uint32_t &i,
                                                   bool byBitIndex) const {
  if (byBitIndex) {
    return indices[i];
  }
  return indices[flag2BitIndex(i)];
}

inline uint32_t
Application::QueueFamilyIndices::flag2BitIndex(const uint32_t &f) {
  uint32_t bit = 0, t = f;
  for (; t > 1; t >>= 1, ++bit)
    ;
  return bit;
}
