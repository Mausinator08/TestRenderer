#pragma once
// 

#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"
#include <iostream>
#include <set>
#include <string>
#include <chrono>
#include <string_view>


// The validation layers that we might use.
const std::vector<const char*> kValidationLayers = {
	"VK_LAYER_LUNARG_standard_validation",

	// Dumps the params and return values to Vulkan API functions. 
	// Useful to see what we're passing to the API and what they're returning.
	//"VK_LAYER_LUNARG_api_dump", 

	"VK_LAYER_LUNARG_core_validation",
	//"VK_LAYER_LUNARG_device_simulation",
	//"VK_LAYER_LUNARG_monitor",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_parameter_validation",
	//"VK_LAYER_LUNARG_screenshot",
	//"VK_LAYER_LUNARG_vktrace",
	//"VK_LAYER_GOOGLE_threading",
	//"VK_LAYER_GOOGLE_unique_objects",	
	//"VK_LAYER_NV_optimus",
	//"VK_LAYER_VALVE_steam_overlay",
	//"VK_LAYER_RENDERDOC_Capture",
};

const std::vector<const char*> kDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Whether we want layer validation or not.
#ifdef Ben_DEBUG
constexpr bool kEnableValidationLayers = true;
#else
constexpr bool kEnableValidationLayers = false;
#endif

constexpr int kWIDTH = 800;
constexpr int kHEIGHT = 600;

using BinaryFileData = std::vector<char>;
BinaryFileData ReadBinaryFile(std::string_view name);

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
void GlfwErrorCallback(int, const char*);

struct QueueFamilyIndices {
	int32_t mGraphicsFamily = -1; // Default: not found
	int32_t mPresentFamily = -1;

	bool IsComplete() {
		return mGraphicsFamily >= 0 && mPresentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR mCapabilities;
	std::vector<VkSurfaceFormatKHR> mFormats;
	std::vector<VkPresentModeKHR> mPresentModes;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec2 coord;

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, coord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	glm::mat4 mModel;
	glm::mat4 mView;
	glm::mat4 mProj;
};

const std::vector<Vertex> gVertices = {
	{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},	// Vert 0: Top left
	{{0.5f, -0.5f, 0.0f},  {1.0f, 0.0f}},	// Vert 1: Top Right
	{{0.5f, 0.5f, 0.0f},   {1.0f, 1.0f}},	// Vert 2: Bottom Right
	{{-0.5f, 0.5f, 0.0f},  {0.0f, 1.0f}},	// Vert 3: Bottom Left
};;

const std::vector<uint16_t> gIndices = {
	0, 1, 2, 2, 3, 0
};

struct TestRenderer {

	// Master init routine
	void Start() {
		InitWindow();
		InitVulkan();
	}

	// Main game loop.
	bool Update() {
		if (glfwWindowShouldClose(mWindow)) {
			return false;
		}

		glfwPollEvents();
		UpdateUniformBuffer();
		DrawFrame();
		return true;
	}

	// Cleanup for shutdown.
	void Stop() {
		// Wait to finish any async operation that might still be going one.
		vkDeviceWaitIdle(mLogicalDevice);

		CleanupSwapChain();

		vkDestroySampler(mLogicalDevice, mTextureSampler, nullptr);
		vkDestroyImageView(mLogicalDevice, mTextureImageView, nullptr);
		vkDestroyImage(mLogicalDevice, mTextureImage, nullptr);
		vkFreeMemory(mLogicalDevice, mTextureImageMemory, nullptr);
		vkDestroyDescriptorSetLayout(mLogicalDevice, mDescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(mLogicalDevice, mDescriptorPool, nullptr);
		vkDestroyBuffer(mLogicalDevice, mIndexBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, mIndexBufferMemory, nullptr);
		vkDestroyBuffer(mLogicalDevice, mVertexBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, mVertexBufferMemory, nullptr);
		vkDestroyBuffer(mLogicalDevice, mUniformBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, mUniformBufferMemory, nullptr);
		vkDestroySemaphore(mLogicalDevice, mRenderFinishedSemaphore, nullptr);
		vkDestroySemaphore(mLogicalDevice, mImageAvailableSemaphore, nullptr);
		vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyDevice(mLogicalDevice, nullptr);
		DestroyDebugReportCallbackEXT(mInstance, mDebugCallback, nullptr);
		vkDestroyInstance(mInstance, nullptr);
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

private:
	// Window initialization stuff.
	void InitWindow() {
		// Initialize GLFW library
		glfwInit();

		glfwSetErrorCallback(GlfwErrorCallback);

		// Don't create an OpenGL context.
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Don't allow resizing of the window.
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Create the window handle.
		mWindow = glfwCreateWindow(kWIDTH, kHEIGHT, "TestRenderer", nullptr, nullptr);

		glfwSetWindowUserPointer(mWindow, this);
		glfwSetWindowSizeCallback(mWindow, TestRenderer::OnWindowResized);
	}

	static void OnWindowResized(GLFWwindow* window, int width, int height) {
		if (width == 0 || height == 0) return;

		TestRenderer* app = reinterpret_cast<TestRenderer*>(glfwGetWindowUserPointer(window));
		app->RecreateSwapChain();
	}

	// Show the Vulkan extensions that are supported by the GPU
	void ShowSupportedExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available Vulkan Extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}
	}

	// Create an instance of the Vulkan library.
	void CreateVulkanInstance() {
		if (kEnableValidationLayers && CheckValidationLayerSupport() == false) {
			throw std::runtime_error("One or more validation-layers are not available.");
		}

		// Some info about our application and also the general Vulkan API we're using.
		// §3.2. Instances
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "BattleEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Information about our required Vulkan extensions.
		auto extensions = GetRequiredExtensions();

		// The arg passed to the instance creation function.
		// §3.2. Instances
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Validation-layers
		if (kEnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
			createInfo.ppEnabledLayerNames = kValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// Actually create the Vulkan instance.
		// §3.2. Instances
		if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
			throw std::runtime_error("Could not create Vulkan Instance.");
		}
	}

	// Choose the pixed depth and format.
	VkSurfaceFormatKHR
		ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		// Just choose the first format, since we didn't find a preffered one.
		return availableFormats[0];
	}

	// Choose the mode that we're going to present the swap-chain (double-buffer/triple-buffer)
	VkPresentModeKHR
		ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}

	// Choose the resolution of the swap-chain images.
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}

		int width, height;
		glfwGetWindowSize(mWindow, &width, &height);
		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	// Check whether the validation-layers that we want enabled are suppored.
	bool CheckValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::cout << "Available validation-layers: \n";
		for (const auto& layerProperties : availableLayers) {
			std::cout << "\t" << layerProperties.layerName << "\n";
		}

		for (auto layerName : kValidationLayers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				std::cerr << "Validation layer not available: " << layerName << "\n";
				return false;
			}
		}

		return true;
	}

	// Get a list of Vulkan extensions that we required for the app.
	auto GetRequiredExtensions() -> std::vector<const char*> {
		std::vector<const char*> extensions;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i = 0; i < glfwExtensionCount; i++) {
			extensions.push_back(glfwExtensions[i]);
		}

		if (kEnableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	// The debug function that will be called by the validation layers if problems are encountered.
	static VKAPI_ATTR VkBool32 VKAPI_CALL
		LayerDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
		std::cerr << "Validation layer: " << msg << std::endl;
		return VK_FALSE;
	}

	// Setup the function that we'll use to get messages from validation layers.
	void SetupDebugCallback() {
		if (kEnableValidationLayers == false) {
			return;
		}

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = LayerDebugCallback;

		if (CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr, &mDebugCallback) != VK_SUCCESS) {
			throw std::runtime_error("Failed to set up Vulkan debug callback.");
		}
	}

	// Select a graphics card to use.
	void PickPhysicalDevice() {
		uint32_t deviceCount = 0;

		// Get the count of how many graphic cards we have.
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find a GPU with Vulkan support.");
		}

		// Grab handles to the physical cards found that support Vulkan.
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

		// Choose the first physical device that suits our needs.
		// Note: It would be best to give each device a score and 
		//       pick the best, based on that score.
		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				mPhysicalDevice = device;
				break;
			}
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU.");
		}
	}

	// Check that a GPU has the capabilities that we need for our app.
	bool IsDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = FindQueueFamilies(device);
		bool extensionsSupported = CheckDeviceExtensionSupport(device);
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	};

	// Check that the GPU supports some extensions that we require.
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	// Find out the types of queue families supported by the graphics card.
	auto FindQueueFamilies(VkPhysicalDevice device) -> QueueFamilyIndices {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;

		// §4.1. Physical Devices
		// Get the amount of queue families supported by the device.
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);


		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// See if this Queue famility support creating graphics.
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.mGraphicsFamily = i;
			}

			// Check that the device can present surface on a window.
			// Some devices are compute only.
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.mPresentFamily = i;
			}

			if (indices.IsComplete()) {
				break;
			}

			++i;
		}

		return indices;
	}

	// Create the logical device, which represents the physical GPU.
	void CreateLogicalDevice() {
		// Find out the queue families that this GPU supports.
		QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

		float queuePriority = 1.0f;

		// Fill in one 'VkDeviceQueueCreateInfo' for each queue family that we want to use.
		// This allows us to specify how many queues within each family to create.
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int32_t> uniqueQueueFamilies = { indices.mGraphicsFamily, indices.mPresentFamily };
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Specify a set of device features that we'll be using.
		// This includes the queue families that we want to use.
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

		if (kEnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
			createInfo.ppEnabledLayerNames = kValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// §4.2.1. Device Creation
		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		// Retrieve handles to the queues (not the queue families).
		constexpr uint32_t queueIndex = 0;
		vkGetDeviceQueue(mLogicalDevice, indices.mGraphicsFamily, queueIndex, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicalDevice, indices.mPresentFamily, queueIndex, &mPresentQueue);
	}

	// Query the details of the GPU's swap-chain support.
	auto QuerySwapChainSupport(VkPhysicalDevice device) -> SwapChainSupportDetails {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.mCapabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.mFormats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.mPresentModes.data());
		}

		return details;
	}

	// Create the Vulkan Surface, based on the Windowing handle, where we'll write the graphics.
	void CreateSurface() {
		if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// Create the image swap chain.
	void CreateSwapChain() {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.mFormats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.mPresentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.mCapabilities);

		// The amount of images supported by the swap chain. 
		// We want, at least, 3 for triple buffering.
		uint32_t imageCount = swapChainSupport.mCapabilities.minImageCount + 1;
		if (swapChainSupport.mCapabilities.maxImageCount > 0 && imageCount > swapChainSupport.mCapabilities.maxImageCount) {
			imageCount = swapChainSupport.mCapabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.mGraphicsFamily, (uint32_t)indices.mPresentFamily };

		// NOTE: Performance:
		// VK_SHARING_MODE_EXCLUSIVE is faster, but requires to careful handling.
		// In the future, if needed for speed, maybe always use VK_SHARING_MODE_EXCLUSIVE.
		// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
		if (indices.mGraphicsFamily != indices.mPresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.mCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain.");
		}

		vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, nullptr);
		mSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, mSwapChainImages.data());

		mSwapChainImageFormat = surfaceFormat.format;
		mSwapChainExtent = extent;
	}

	// Create the ImageViews that give us access to image buffers.
	void CreateImageViews() {
		mSwapChainImageViews.resize(mSwapChainImages.size());

		for (uint32_t i = 0; i < mSwapChainImages.size(); i++) {
			mSwapChainImageViews[i] = CreateImageView(mSwapChainImages[i], mSwapChainImageFormat);
		}
	}

	VkShaderModule CreateShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(mLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void CreateGraphicsPipeline() {
		auto vertShaderCode = ReadBinaryFile("Shaders/Vert.spv");
		auto fragShaderCode = ReadBinaryFile("Shaders/Frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)mSwapChainExtent.width;
		viewport.height = (float)mSwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		if (vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = mPipelineLayout;
		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(mLogicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(mLogicalDevice, vertShaderModule, nullptr);
	}

	void CreateRenderPass() {
		// An attachment description describes the properties of an attachment, including its format, 
		// sample count, and how its contents are treated at the beginning and end of each render pass instance.
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = mSwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// A subpass description describes the subset of attachments that are involved in the execution of a subpass.
		// A subpass uses an attachment if the attachment is a color, depth/stencil, resolve, or input attachment for 
		// that subpass (as determined by the pColorAttachments, pDepthStencilAttachment, pResolveAttachments, and 
		// pInputAttachments members of VkSubpassDescription, respectively).
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// Subpass dependencies describe execution and memory dependencies between subpasses.
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(mLogicalDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render pass.");
		}
	}

	void CreateFramebuffers() {
		mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				mSwapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = mSwapChainExtent.width;
			framebufferInfo.height = mSwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(mLogicalDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void CreateCommandPool() {
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.mGraphicsFamily;

		if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void CreateCommandBuffers() {
		mCommandBuffers.resize(mSwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

		if (vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < mCommandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass;
			renderPassInfo.framebuffer = mSwapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mSwapChainExtent;

			VkClearValue clearColor = { 0.2f, 0.2f, 0.2f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
			VkBuffer vertexBuffers[] = { mVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, nullptr);
			vkCmdBindIndexBuffer(mCommandBuffers[i], mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(mCommandBuffers[i], static_cast<uint32_t>(gIndices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(mCommandBuffers[i]);

			if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer.");
			}
		}
	}

	void CreateSemaphores() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		auto res1 = vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphore);
		auto res2 = vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphore);
		if (res1 != VK_SUCCESS || res2 != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphores.");
		}
	}

	void CleanupSwapChain() {
		for (size_t i = 0; i < mSwapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(mLogicalDevice, mSwapChainFramebuffers[i], nullptr);
		}

		vkFreeCommandBuffers(mLogicalDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
		vkDestroyPipeline(mLogicalDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
		vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
			vkDestroyImageView(mLogicalDevice, mSwapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);
	}

	void RecreateSwapChain() {
		vkDeviceWaitIdle(mLogicalDevice);

		CleanupSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandBuffers();
	}


	void DrawFrame() {
		uint32_t imageIndex;
		constexpr uint64_t noTimeout = std::numeric_limits<uint64_t>::max();
		VkResult result = vkAcquireNextImageKHR(mLogicalDevice, mSwapChain, noTimeout, mImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore };
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;

		if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image.");
		}

		vkQueueWaitIdle(mPresentQueue);
	}

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(mLogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer.");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mLogicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		// Per:
		// https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
		// We're not supposed to actually call vkAllocateMemory for every individual buffer.
		// The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical 
		// device limit, which may be as low as 4096 even on high end hardware like an NVIDIA GTX 1080. 
		// The right way to allocate memory for a large number of objects at the same time is to create a 
		// custom allocator that splits up a single allocation among many different objects by using the 
		// offset parameters that we've seen in many functions.
		// We can either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided 
		// by the GPUOpen initiative. 
		if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate vertex buffer memory.");
		}

		vkBindBufferMemory(mLogicalDevice, buffer, bufferMemory, 0);
	}

	// Create a region of memory in the graphics card where we'll store the vertex data.
	void CreateVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(gVertices[0]) * gVertices.size();

		// Create staging buffer in the GPU, to which we can transfer
		// data from our application.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy the data to the staging buffer.
		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, gVertices.data(), (size_t)bufferSize);
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		// Create the GPU logical buffer. We cannot tranfer data directly to this buffer,
		// but we can copy from the staging buffer to this buffer.
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);
		CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
	}

	// NOTE: This is verify similar to the CreateVertexBuffer() function.
	//       These 2 functions can probably be combined into one.
	void CreateIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(gIndices[0]) * gIndices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, gIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

		CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
	}

	void CreateUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffer, mUniformBufferMemory);
	}

	// NOTE: There are optimization oportunities here:
	//		1. Use a command pool for copying of buffers.
	//		2. If multiple transfers are needed, instead of waiting with vkQueueWaitIdle, we
	//		   can use vkWaitForFences to schedule multiple transfers and then do one wait.
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);


		EndSingleTimeCommands(commandBuffer);
	}


	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		/////////
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}
		////////////

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer BeginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &commandBuffer);
	}

	void CreateDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0; //  The binding used in the shader program.
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // We're using this description only in the vertex shader.
		uboLayoutBinding.pImmutableSamplers = nullptr; // This is optional, and it's related to image sampling descriptors.

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(mLogicalDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}
	}

	void CreateDescriptorPool() {
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(mLogicalDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool.");
		}
	}

	void CreateDescriptorSet() {
		VkDescriptorSetLayout layouts[] = { mDescriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(mLogicalDevice, &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor Set.");
		}

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = mUniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = mTextureImageView;
		imageInfo.sampler = mTextureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = mDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = mDescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(mLogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(mLogicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mLogicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(mLogicalDevice, image, imageMemory, 0);
	}

	void CreateTextureImageView() {
		mTextureImageView = CreateImageView(mTextureImage, VK_FORMAT_R8G8B8A8_UNORM);
	}

	VkImageView CreateImageView(VkImage image, VkFormat format) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(mLogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void CreateTextureSampler() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(mLogicalDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void CreateTextureImage();



	void UpdateUniformBuffer() {
		static auto startTime = std::chrono::high_resolution_clock::now();

		// Time, in seconds.
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		UniformBufferObject ubo = {};

		auto identMat = glm::mat4(1.0f);
		auto rotAmount = 0.0f;
		auto rotAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		ubo.mModel = glm::rotate(identMat, rotAmount, rotAxis);

		float eyeXPos = 0.0f;// time * 1.0f;
		float eyeYPos = 0.0f; // time * 1.0f;
		// THE MAIN FIX - John P Maus: Start the eye z position far away from the t-shirt along the negative z axis, then it will zoom in like expected.
		// If starting eyeZPos is 0.0f, then when you zoom futher, because you are constantly looking at position 0.0f, the camera is moving away from
		// The t-shirt on the positive z axis, but still looking at 0.0f on the z axis so it seems like it's getting farther away. The image displays on
		// the quad no matter which side you are looking at, making the camera orientation seem as if you were on the negative z axis.
		float eyeZPos = (time * 1.0f) - 10.0f;

		auto eyePosition = glm::vec3(eyeXPos, eyeYPos, eyeZPos);
		auto centerPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		// ALT FIX - John P Maus: Now that the image is further back, if you don't want the image to stay in view, have the eyeZPos always looking 1.0f ahead
		// on the z axis, and it's like a first person camera at that point, like so:
		// auto centerPosition = glm::vec3(0.0f, 0.0f, eyeZPos + 1.0f);
		auto upAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		ubo.mView = glm::lookAt(eyePosition, centerPosition, upAxis);

		auto verticalFOV = glm::radians(45.0f);
		auto aspectRatio = mSwapChainExtent.width / (float)mSwapChainExtent.height;
		auto nearPlane = 0.1f;
		auto farPlane = 100.0f;
		ubo.mProj = glm::perspective(verticalFOV, aspectRatio, nearPlane, farPlane);

		// We're not OpenGL, to flip the Y coordinate.
		//ubo.mProj[1][1] *= -1;

		// NOTE: This is not the most efficient way to pass frequently changing data.
		//	     A more efficient way to pass a small buffer of data to shaders are *push constants*.
		//		 https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
		void* data;
		vkMapMemory(mLogicalDevice, mUniformBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(mLogicalDevice, mUniformBufferMemory);
	}

	// Init Vulkan specific stuff.
	void InitVulkan() {
		//ShowSupportedExtensions();
		CreateVulkanInstance();		// Ok
		SetupDebugCallback(); // Ok
		CreateSurface();		// Ok
		PickPhysicalDevice();	// Ok
		CreateLogicalDevice(); // Ok
		CreateSwapChain();		// Ok
		CreateImageViews();		// Ok
		CreateRenderPass();     // Ok
		CreateDescriptorSetLayout(); // Ok
		CreateGraphicsPipeline(); // Ok
		CreateFramebuffers();		// Ok
		CreateCommandPool();       // Ok
		CreateTextureImage();		// Ok
		CreateTextureImageView();	// Ok
		CreateTextureSampler();		// Ok
		CreateVertexBuffer(); // Ok
		CreateIndexBuffer();  // Ok
		CreateUniformBuffer(); // Ok
		CreateDescriptorPool(); // Ok
		CreateDescriptorSet(); // Partial
		CreateCommandBuffers();
		CreateSemaphores(); // Ok
	}

private:
	GLFWwindow* mWindow;
	VkInstance mInstance; // Represents the main connection to the Vulkan library.
	VkDebugReportCallbackEXT mDebugCallback;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE; // Represents the actual graphics card that we end up using. It is implicitly destroyed when VkInstance is destroyed.
	VkDevice mLogicalDevice; // We interact with a physical device via a logical device.
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkSurfaceKHR mSurface;
	VkSwapchainKHR mSwapChain;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	VkDescriptorPool mDescriptorPool;
	VkDescriptorSet mDescriptorSet;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout;
	VkRenderPass mRenderPass;
	VkPipeline mGraphicsPipeline;
	VkCommandPool mCommandPool;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	VkBuffer mUniformBuffer;
	VkDeviceMemory mUniformBufferMemory;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	std::vector<VkCommandBuffer> mCommandBuffers;
	VkSemaphore mImageAvailableSemaphore;
	VkSemaphore mRenderFinishedSemaphore;

	VkSampler mTextureSampler;
	VkImageView mTextureImageView;
	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
};