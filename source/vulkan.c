#ifdef __linux__

#include "vulkan.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <wayland-client.h>
#include <X11/Xlib.h>
#include <vulkan/vulkan_wayland.h>
#include <vulkan/vulkan_xlib.h>
#include <xkbcommon/xkbcommon.h>
#include <meow_utilities/include/meow_math.h>
#include <meow_utilities/include/log.h>
#include <meow_utilities/include/fileSystem.h>

#define MAX_FRAMES_IN_FLIGHT 2

void deinitializeVulkanContext(struct vulkanContext *vulkanContext);

const char* vulkanResultToString(VkResult result) {
	switch (result) {
		case VK_SUCCESS:
			return "VK_SUCCESS";
		case VK_NOT_READY:
			return "VK_NOT_READY";
		case VK_TIMEOUT:
			return "VK_TIMEOUT";
		case VK_EVENT_SET:
			return "VK_EVENT_SET";
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET";
		case VK_INCOMPLETE:
			return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
		    return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:
			return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION:
			return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_PIPELINE_COMPILE_REQUIRED:
			return "VK_PIPELINE_COMPILE_REQUIRED";
		case VK_ERROR_NOT_PERMITTED:
			return "VK_ERROR_NOT_PERMITTED";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:
			return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
			return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
		case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
		case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
		case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
		case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
		case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:
			return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:
			return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:
			return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
			return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
		case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
			return "VK_ERROR_COMPRESSION _EXHAUSTED_EXT";
		case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
			return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
		case VK_PIPELINE_BINARY_MISSING_KHR:
			return "VK_PIPELINE_BINARY_MISSING_KHR";
		case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
			return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
		case VK_RESULT_MAX_ENUM:
			return "VK_RESULT_MAX_ENUM";
        default:
            return "UNKNOWN";
	}
}

#define HANDLE_VULKAN_RESULT(result,errorMessage,line,returnValue) { \
    if (result) { \
        meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"%s (%s, %d)",errorMessage,vulkanResultToString(result),line); \
        return returnValue; \
    } \
}

static VkBool32 vulkanDebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,void *pUserData) {
    meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Validation layer - %s",pCallbackData->pMessage);
    return VK_FALSE;
}

static const VkShaderModule createShaderModule(char *path,struct vulkanContext *vulkanContext) {
    int32_t fileContentSize = 0;
    char *fileContents = meow_readFile(path,1,&fileContentSize);
    
    if (!fileContents) {
        meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"%s (%d)",strerror(errno),__LINE__);
        deinitializeVulkanContext(vulkanContext);
        return NULL;
    }
    
    VkShaderModuleCreateInfo shaderModuleCreateInformation = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pCode = (uint32_t *)fileContents,
        .codeSize = fileContentSize
    };
    
    VkShaderModule shaderModule = {};
    
    HANDLE_VULKAN_RESULT(vkCreateShaderModule(vulkanContext->device,&shaderModuleCreateInformation,NULL,&shaderModule),"Failed to create Vulkan shader module",__LINE__,NULL);
    
    return shaderModule;
}

static void destroySwapchain(struct vulkanContext *vulkanContext) {
    for (uint32_t swapchainImageIndex = 0; swapchainImageIndex < vulkanContext->swapchainImageCount; swapchainImageIndex++) {
        vkDestroyImageView(vulkanContext->device,vulkanContext->swapchainImageViews[swapchainImageIndex],NULL);
        vkDestroyFramebuffer(vulkanContext->device,vulkanContext->swapchainFramebuffers[swapchainImageIndex],NULL);
    }
    
    vkDestroySwapchainKHR(vulkanContext->device,vulkanContext->swapchain,NULL);
    
    free(vulkanContext->swapchainImages);
    vulkanContext->swapchainImages = NULL;
    
    free(vulkanContext->swapchainImageViews);
    vulkanContext->swapchainImageViews = NULL;
    
    free(vulkanContext->swapchainFramebuffers);
    vulkanContext->swapchainFramebuffers = NULL;
}

const _Bool createSwapchain(struct vulkanContext *vulkanContext,meow_vector2 imageSize) {
    vkDeviceWaitIdle(vulkanContext->device);
    
    destroySwapchain(vulkanContext);
    
    HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanContext->physicalDevice,vulkanContext->surface,&vulkanContext->physicalDeviceSurfaceCapabilities),"Failed to get Vulkan physical device surface capabilities",__LINE__,1);
    
    // Create Vulkan swapchain and images
    {
        vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.width = meow_clamp(imageSize.x,vulkanContext->physicalDeviceSurfaceCapabilities.minImageExtent.width,vulkanContext->physicalDeviceSurfaceCapabilities.maxImageExtent.width);
        vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.height = meow_clamp(imageSize.y,vulkanContext->physicalDeviceSurfaceCapabilities.minImageExtent.height,vulkanContext->physicalDeviceSurfaceCapabilities.maxImageExtent.height);
        
        uint32_t swapchainMinimumImagesCount = vulkanContext->physicalDeviceSurfaceCapabilities.minImageCount + 1;
        
        if (vulkanContext->physicalDeviceSurfaceCapabilities.maxImageCount > 0 && swapchainMinimumImagesCount > vulkanContext->physicalDeviceSurfaceCapabilities.maxImageCount) {
            swapchainMinimumImagesCount = vulkanContext->physicalDeviceSurfaceCapabilities.maxImageCount;
        }
        
        VkSwapchainCreateInfoKHR swapchainCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = vulkanContext->surface,
            .minImageCount = swapchainMinimumImagesCount,
            .imageFormat = vulkanContext->physicalDeviceSurfaceFormat.format,
            .imageColorSpace = vulkanContext->physicalDeviceSurfaceFormat.colorSpace,
            .imageExtent = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = vulkanContext->physicalDeviceSurfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = vulkanContext->physicalDeviceSurfacePresentationMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };
        
        const uint32_t queueFamilyIndicies[] = {
            vulkanContext->graphicsQueueFamilyIndex,
            vulkanContext->presentationQueueFamilyIndex
        };
        
        if (vulkanContext->graphicsQueueFamily != vulkanContext->presentationQueueFamily) {
            swapchainCreateInformation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInformation.queueFamilyIndexCount = 2;
            swapchainCreateInformation.pQueueFamilyIndices = queueFamilyIndicies;
        } else {
            swapchainCreateInformation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        
        HANDLE_VULKAN_RESULT(vkCreateSwapchainKHR(vulkanContext->device,&swapchainCreateInformation,NULL,&vulkanContext->swapchain),"Failed to create Vulkan swapchain",__LINE__,1);
        
        HANDLE_VULKAN_RESULT(vkGetSwapchainImagesKHR(vulkanContext->device,vulkanContext->swapchain,&vulkanContext->swapchainImageCount,NULL),"Failed to get Vulkan swapchain images count",__LINE__,1);
        
        vulkanContext->swapchainImages = malloc(sizeof(VkImage) * vulkanContext->swapchainImageCount);
        
        HANDLE_VULKAN_RESULT(vkGetSwapchainImagesKHR(vulkanContext->device,vulkanContext->swapchain,&vulkanContext->swapchainImageCount,vulkanContext->swapchainImages),"Failed to get Vulkan swapchain images",__LINE__,1);
    }
    
    // Create Vulkan swapchain image views
    {
        vulkanContext->swapchainImageViews = malloc(sizeof(VkImageView) * vulkanContext->swapchainImageCount);
        
        for (uint32_t swapchainImageIndex = 0; swapchainImageIndex < vulkanContext->swapchainImageCount; swapchainImageIndex++) {
            VkImage swapchainImage = vulkanContext->swapchainImages[swapchainImageIndex];
            
            VkImageViewCreateInfo imageViewCreateInformation = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapchainImage,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = vulkanContext->physicalDeviceSurfaceFormat.format,
                .components = (VkComponentMapping){
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1
            };
            
            HANDLE_VULKAN_RESULT(vkCreateImageView(vulkanContext->device,&imageViewCreateInformation,NULL,&vulkanContext->swapchainImageViews[swapchainImageIndex]),"Failed to create Vulkan swapchain image view",__LINE__,1);
        }
    }
    
    // Create Vulkan framebuffers
    {
        vulkanContext->swapchainFramebuffers = malloc(sizeof(VkFramebuffer) * vulkanContext->swapchainImageCount);
        
        for (uint32_t swapchainImageIndex = 0; swapchainImageIndex < vulkanContext->swapchainImageCount; swapchainImageIndex++) {
            VkImageView attachments[] = {vulkanContext->swapchainImageViews[swapchainImageIndex]};
            
            VkFramebufferCreateInfo framebufferCreateInformation = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanContext->renderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.width,
                .height = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.height,
                .layers = 1
            };
            
            HANDLE_VULKAN_RESULT(vkCreateFramebuffer(vulkanContext->device,&framebufferCreateInformation,NULL,&vulkanContext->swapchainFramebuffers[swapchainImageIndex]),"Failed to create Vulkan framebuffer",__LINE__,1);
        }
    }
    
    return 0;
}

const _Bool initializeVulkanContext(struct vulkanContext *vulkanContext,struct wl_surface *surface,struct wl_display *display,Window *x11Window,meow_vector2 imageSize) {
    #ifndef NDEBUG
        const char* layerNames[] = {
            "VK_LAYER_KHRONOS_validation"
        };
    #endif
    
    // Create Vulkan instance
    {
        #ifdef NDEBUG
            const char *instanceExtensions[] = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
                VK_KHR_XLIB_SURFACE_EXTENSION_NAME
            };
        #else
            const char *instanceExtensions[] = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
                VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
            };
        #endif
        
        // Validate Vulkan instance extension support
        {
            uint32_t vulkanInstanceExtensionsCount = 0;
            HANDLE_VULKAN_RESULT(vkEnumerateInstanceExtensionProperties(NULL,&vulkanInstanceExtensionsCount,NULL),"Failed to get Vulkan instance extensions count",__LINE__,1);
            
            VkExtensionProperties *vulkanInstanceExtensions = malloc(sizeof(VkExtensionProperties) * vulkanInstanceExtensionsCount);
            HANDLE_VULKAN_RESULT(vkEnumerateInstanceExtensionProperties(NULL,&vulkanInstanceExtensionsCount,vulkanInstanceExtensions),"Failed to get Vulkan instance extensions",__LINE__,1);
            
            for (uint8_t instanceExtensionIndex = 0; instanceExtensionIndex < sizeof(instanceExtensions) / sizeof(char *); instanceExtensionIndex++) {
                const char *instanceExtensionName = instanceExtensions[instanceExtensionIndex];
                _Bool instanceExtensionFound = 0;
                
                for (uint8_t vulkanInstanceExtensionIndex = 0; vulkanInstanceExtensionIndex < vulkanInstanceExtensionsCount; vulkanInstanceExtensionIndex++) {
                    char *vulkanInstanceExtensionName = vulkanInstanceExtensions[vulkanInstanceExtensionIndex].extensionName;
                    
                    if (!strcmp(instanceExtensionName,vulkanInstanceExtensionName)) {
                        instanceExtensionFound = 1;
                        break;
                    }
                }
                
                if (instanceExtensionFound) {
                    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Vulkan instance extension \"%s\" is supported",instanceExtensionName);
                } else {
                    meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Vulkan instance extension \"%s\" is not supported",instanceExtensionName);
                    return 1;
                }
            }
        }
        
        VkApplicationInfo applicationInformation = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .apiVersion = VK_API_VERSION_1_4
        };
        
        VkInstanceCreateInfo instanceCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &applicationInformation,
            .enabledExtensionCount = sizeof(instanceExtensions) / sizeof(char *),
            .ppEnabledExtensionNames = instanceExtensions
        };
        
        #ifndef NDEBUG
            instanceCreateInformation.enabledLayerCount = sizeof(layerNames) / sizeof(char *);
            instanceCreateInformation.ppEnabledLayerNames = layerNames;
        #endif
        
        HANDLE_VULKAN_RESULT(vkCreateInstance(&instanceCreateInformation,NULL,&vulkanContext->instance),"Failed to create Vulkan instance",__LINE__,1);
    }
    
    // Create Vulkan debug messenger
    {
        #ifndef NDEBUG
            VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInformation = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = vulkanDebugMessengerCallback
            };
            
            PFN_vkCreateDebugUtilsMessengerEXT createDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->instance,"vkCreateDebugUtilsMessengerEXT");
            
            if (createDebugMessenger) {
                HANDLE_VULKAN_RESULT(createDebugMessenger(vulkanContext->instance,&debugMessengerCreateInformation,NULL,&vulkanContext->debugMessenger),"Failed to initialize debug messenger",__LINE__,1);
            } else {
                meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to create debug messenger initializer");
                return 1;
            }
        #endif
    }
    
    // Create Vulkan surface
    {
        VkWaylandSurfaceCreateInfoKHR waylandSurfaceCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .surface = surface,
            .display = display
        };
        
        HANDLE_VULKAN_RESULT(vkCreateWaylandSurfaceKHR(vulkanContext->instance,&waylandSurfaceCreateInformation,NULL,&vulkanContext->surface),"Failed to create Vulkan surface",__LINE__,1);
    }
    
    // Select Vulkan physical device, create logical device. and create graphics queue
    {
        uint32_t physicalDeviceCount = 0;
        HANDLE_VULKAN_RESULT(vkEnumeratePhysicalDevices(vulkanContext->instance,&physicalDeviceCount,NULL),"Failed to get Vulkan physical device count",__LINE__,1);
        
        VkPhysicalDevice physicalDevices[physicalDeviceCount] = {};
        HANDLE_VULKAN_RESULT(vkEnumeratePhysicalDevices(vulkanContext->instance,&physicalDeviceCount,physicalDevices),"Failed to get Vulkan physical devices",__LINE__,1);
        
        VkPhysicalDevice mostSuitablePhysicalDevice = {};
        VkPhysicalDeviceProperties mostSuitablePhysicalDeviceProperties = {};
        VkPhysicalDeviceFeatures mostSutablePhysicalDeviceFeatures = {};
        uint16_t mostSuitablePhysicalDeviceSuitability = 0;
        uint32_t mostSuitablePhysicalDeviceGraphicsQueueFamilyIndex = 0;
        uint32_t mostSuitablePhysicalDevicePresentationQueueFamilyIndex = 0;
        
        const char *physicalDeviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        
        for (uint8_t physicalDeviceIndex = 0; physicalDeviceIndex < physicalDeviceCount; physicalDeviceIndex++) {
            VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];
            
            VkPhysicalDeviceProperties physicalDeviceProperties = {};
            vkGetPhysicalDeviceProperties(physicalDevice,&physicalDeviceProperties);
            
            meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Checking if \"%s\" meets requirements...",physicalDeviceProperties.deviceName);
            
            VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
            vkGetPhysicalDeviceFeatures(physicalDevice,&physicalDeviceFeatures);
            
            uint32_t physicalDeviceGraphicsQueueFamilyIndex = 0;
            _Bool physicalDeviceGraphicsQueueFamilyIndexExists = 0;
            uint32_t physicalDevicePresentationQueueFamilyIndex = 0;
            _Bool physicalDevicePresentationQueueFamilyIndexExists = 0;
            
            // Vailidate required Vulkan queue families
            {
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueFamilyCount,NULL);
                
                VkQueueFamilyProperties queueFamilies[queueFamilyCount] = {};
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueFamilyCount,queueFamilies);
                
                for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
                    if (!physicalDeviceGraphicsQueueFamilyIndexExists) {
                        if (queueFamilies[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                            physicalDeviceGraphicsQueueFamilyIndexExists = 1;
                            physicalDeviceGraphicsQueueFamilyIndex = queueFamilyIndex;
                        }
                    }
                    
                    // Validate Vulkan presentation extension support
                    if (!physicalDevicePresentationQueueFamilyIndexExists) {
                        VkBool32 physicalDevicePresentationSupport = VK_FALSE;
                        
                        HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,queueFamilyIndex,vulkanContext->surface,&physicalDevicePresentationSupport),"Failed to get Vulkan physical device presentation support",__LINE__,1);
                        
                        if (physicalDevicePresentationSupport) {
                            physicalDevicePresentationQueueFamilyIndexExists = 1;
                            physicalDevicePresentationQueueFamilyIndex = queueFamilyIndex;
                        }
                    }
                    
                    // Validate Vulkan queue families
                    {
                        HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,vulkanContext->surface,&vulkanContext->physicalDeviceSurfaceCapabilities),"Failed to get Vulkan physical device surface capabilities",__LINE__,1);
                        
                        HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,vulkanContext->surface,&vulkanContext->physicalDeviceSurfaceFormatCount,NULL),"Failed to get Vulkan physical device surface formats count",__LINE__,1);
                        
                        if (vulkanContext->physicalDeviceSurfaceFormats) {
                            free(vulkanContext->physicalDeviceSurfaceFormats);
                        }
                        
                        vulkanContext->physicalDeviceSurfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * vulkanContext->physicalDeviceSurfaceFormatCount);
                        
                        HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,vulkanContext->surface,&vulkanContext->physicalDeviceSurfaceFormatCount,vulkanContext->physicalDeviceSurfaceFormats),"Failed to get Vulkan physical device surface formats",__LINE__,1);
                        
                        HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,vulkanContext->surface,&vulkanContext->physicalDeviceSurfacePresentationModeCount,NULL),"Failed to get Vulkan physical device surface presentation modes count",__LINE__,1);
                        
                        if (vulkanContext->physicalDeviceSurfacePresentationModes) {
                            free(vulkanContext->physicalDeviceSurfacePresentationModes);
                        }
                        
                        vulkanContext->physicalDeviceSurfacePresentationModes = malloc(sizeof(VkPresentModeKHR) * vulkanContext->physicalDeviceSurfacePresentationModeCount);
                        
                        HANDLE_VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,vulkanContext->surface,&vulkanContext->physicalDeviceSurfacePresentationModeCount,vulkanContext->physicalDeviceSurfacePresentationModes),"Failed to get Vulkan physical device surface presentation modes",__LINE__,1);
                    }
                    
                    if (physicalDeviceGraphicsQueueFamilyIndexExists && physicalDevicePresentationQueueFamilyIndexExists && vulkanContext->physicalDeviceSurfaceFormatCount && vulkanContext->physicalDeviceSurfacePresentationModeCount) {
                        break;
                    }
                }
            }
            
            _Bool physicalDeviceExtensionsSupported = 1;
            
            // Validate Vulkan physical device extension support
            {
                uint32_t vulkanPhysicalDeviceExtensionsCount = 0;
                HANDLE_VULKAN_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice,NULL,&vulkanPhysicalDeviceExtensionsCount,NULL),"Failed to get Vulkan physical device extensions count",__LINE__,1);
                
                VkExtensionProperties *vulkanPhysicalDeviceExtensions = malloc(sizeof(VkExtensionProperties) * vulkanPhysicalDeviceExtensionsCount);
                HANDLE_VULKAN_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice,NULL,&vulkanPhysicalDeviceExtensionsCount,vulkanPhysicalDeviceExtensions),"Failed to get Vulkan physical device extensions",__LINE__,1);
                
                for (uint8_t physicalDeviceExtensionIndex = 0; physicalDeviceExtensionIndex < sizeof(physicalDeviceExtensions) / sizeof(char *); physicalDeviceExtensionIndex++) {
                    const char *physicalDeviceExtensionName = physicalDeviceExtensions[physicalDeviceExtensionIndex];
                    _Bool physicalDeviceExtensionFound = 0;
                    
                    for (uint8_t vulkanPhysicalDeviceExtensionIndex = 0; vulkanPhysicalDeviceExtensionIndex < vulkanPhysicalDeviceExtensionsCount; vulkanPhysicalDeviceExtensionIndex++) {
                        char *vulkanPhysicalDeviceExtensionName = vulkanPhysicalDeviceExtensions[vulkanPhysicalDeviceExtensionIndex].extensionName;
                        
                        if (!strcmp(physicalDeviceExtensionName,vulkanPhysicalDeviceExtensionName)) {
                            physicalDeviceExtensionFound = 1;
                            break;
                        }
                    }
                    
                    if (physicalDeviceExtensionFound) {
                        meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Vulkan physical device extension \"%s\" is supported",physicalDeviceExtensionName);
                    } else {
                        meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Vulkan physical device extension \"%s\" is not supported",physicalDeviceExtensionName);
                        physicalDeviceExtensionsSupported = 0;
                        break;
                    }
                }
            }
            
            if (physicalDeviceFeatures.geometryShader && physicalDeviceGraphicsQueueFamilyIndexExists && physicalDevicePresentationQueueFamilyIndexExists && physicalDeviceExtensionsSupported) {
                uint16_t physicalDeviceSuitability = 0;
                
                if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    physicalDeviceSuitability += 1000;
                }
                
                physicalDeviceSuitability += physicalDeviceProperties.limits.maxImageDimension2D;
                
                if (physicalDeviceSuitability > mostSuitablePhysicalDeviceSuitability) {
                    mostSuitablePhysicalDevice = physicalDevice;
                    mostSuitablePhysicalDeviceProperties = physicalDeviceProperties;
                    mostSuitablePhysicalDeviceGraphicsQueueFamilyIndex = physicalDeviceGraphicsQueueFamilyIndex;
                    mostSuitablePhysicalDevicePresentationQueueFamilyIndex = physicalDevicePresentationQueueFamilyIndex;
                    mostSutablePhysicalDeviceFeatures = physicalDeviceFeatures;
                }
            }
        }
        
        vulkanContext->physicalDevice = mostSuitablePhysicalDevice;
        
        meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Selected device \"%s\" as Vulkan physical device",mostSuitablePhysicalDeviceProperties.deviceName);
        
        float deviceQueuePriority = 1;
        
        VkDeviceQueueCreateInfo deviceQueueCreateInformations[] = {
            (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = mostSuitablePhysicalDeviceGraphicsQueueFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = &deviceQueuePriority
            },
            (VkDeviceQueueCreateInfo){
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = mostSuitablePhysicalDevicePresentationQueueFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = &deviceQueuePriority
            }
        };
        
        VkDeviceCreateInfo deviceCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pQueueCreateInfos = deviceQueueCreateInformations,
            .queueCreateInfoCount = 1,
            .pEnabledFeatures = &mostSutablePhysicalDeviceFeatures,
            .ppEnabledExtensionNames = physicalDeviceExtensions,
            .enabledExtensionCount = sizeof(physicalDeviceExtensions) / sizeof(char *),
            .enabledLayerCount = 0
        };
        
        #ifndef NDEBUG
            deviceCreateInformation.ppEnabledLayerNames = layerNames;
            deviceCreateInformation.enabledLayerCount = sizeof(layerNames) / sizeof(char *);
        #endif
        
        HANDLE_VULKAN_RESULT(vkCreateDevice(vulkanContext->physicalDevice,&deviceCreateInformation,NULL,&vulkanContext->device),"Failed to create Vulkan device",__LINE__,1);
        
        vulkanContext->graphicsQueueFamilyIndex = mostSuitablePhysicalDeviceGraphicsQueueFamilyIndex;
        vkGetDeviceQueue(vulkanContext->device,vulkanContext->graphicsQueueFamilyIndex,0,&vulkanContext->graphicsQueueFamily);
        vulkanContext->presentationQueueFamilyIndex = mostSuitablePhysicalDevicePresentationQueueFamilyIndex;
        vkGetDeviceQueue(vulkanContext->device,vulkanContext->presentationQueueFamilyIndex,0,&vulkanContext->presentationQueueFamily);
    }
    
    // Select Vulkan queue families
    {
        vulkanContext->physicalDeviceSurfaceFormat = vulkanContext->physicalDeviceSurfaceFormats[0];
        
        // Select Vulkan surface format
        for (uint32_t physicalDeviceSurfaceFormatsIndex = 0; physicalDeviceSurfaceFormatsIndex < vulkanContext->physicalDeviceSurfaceFormatCount; physicalDeviceSurfaceFormatsIndex++) {
            VkSurfaceFormatKHR physicalDeviceSurfaceFormat = vulkanContext->physicalDeviceSurfaceFormats[physicalDeviceSurfaceFormatsIndex];
            
            if (physicalDeviceSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && physicalDeviceSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                vulkanContext->physicalDeviceSurfaceFormat = physicalDeviceSurfaceFormat;
                break;
            }
        }
        
        vulkanContext->physicalDeviceSurfacePresentationMode = VK_PRESENT_MODE_FIFO_KHR;
        
        // Select Vulkan surface presentation mode
        for (uint32_t physicalDeviceSurfacePresentationModesIndex = 0; physicalDeviceSurfacePresentationModesIndex < vulkanContext->physicalDeviceSurfacePresentationModeCount; physicalDeviceSurfacePresentationModesIndex++) {
            VkPresentModeKHR physicalDeviceSurfacePresentationMode = vulkanContext->physicalDeviceSurfacePresentationModes[physicalDeviceSurfacePresentationModesIndex];
            
            if (physicalDeviceSurfacePresentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                vulkanContext->physicalDeviceSurfacePresentationMode = physicalDeviceSurfacePresentationMode;
                break;
            }
        }
    }
    
    // Create Vulkan graphics pipeline
    {
        VkShaderModule vertexShaderModule = createShaderModule("shaders/shader.vert.spv",vulkanContext);
        
        if (!vertexShaderModule) {
            return 1;
        }
        
        VkShaderModule fragmentShaderModule = createShaderModule("shaders/shader.frag.spv",vulkanContext);
        
        if (!fragmentShaderModule) {
            return 1;
        }
        
        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main"
        };
        
        VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main"
        };
        
        vulkanContext->shaderModulesCount = 2;
        
        vulkanContext->shaderModules = malloc(sizeof(VkShaderModule) * vulkanContext->shaderModulesCount);
        vulkanContext->shaderModules[0] = vertexShaderModule;
        vulkanContext->shaderModules[1] = fragmentShaderModule;
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInformation,fragmentShaderStageCreateInformation};
        
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = NULL,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = NULL
        };
        
        VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
        
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
            .pDynamicStates = dynamicStates
        };
        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };
        
        vulkanContext->viewport = (VkViewport){
            .x = 0,
            .y = 0,
            .width = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.width,
            .height = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.height,
            .minDepth = 0,
            .maxDepth = 1
        };
        
        vulkanContext->scissor = (VkRect2D){
            .offset = 0,
            .extent = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent
        };
        
        VkPipelineViewportStateCreateInfo viewportStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &vulkanContext->viewport,
            .scissorCount = 1,
            .pScissors = &vulkanContext->scissor
        };
        
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0,
            .depthBiasClamp = 0,
            .depthBiasSlopeFactor = 0
        };
        
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .sampleShadingEnable = VK_FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1,
            .pSampleMask = NULL,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };
        
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD
        };
        
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState,
            .blendConstants[0] = 0,
            .blendConstants[1] = 0,
            .blendConstants[2] = 0,
            .blendConstants[3] = 0
        };
        
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = NULL,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = NULL
        };
        
        HANDLE_VULKAN_RESULT(vkCreatePipelineLayout(vulkanContext->device,&pipelineLayoutCreateInformation,NULL,&vulkanContext->pipelineLayout),"Failed to create Vulkan pipeline layout",__LINE__,1);
        
        VkAttachmentDescription attachmentDescription = {
            .format = vulkanContext->physicalDeviceSurfaceFormat.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
        
        VkAttachmentReference colorAttachmentReference = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        
        VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentReference
        };
        
        VkSubpassDependency subpassDependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        };
        
        VkRenderPassCreateInfo renderPassCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &attachmentDescription,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency
        };
        
        HANDLE_VULKAN_RESULT(vkCreateRenderPass(vulkanContext->device,&renderPassCreateInformation,NULL,&vulkanContext->renderPass),"Failed to create Vulkan render pass",__LINE__,1);
        
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputStateCreateInformation,
            .pInputAssemblyState = &inputAssemblyStateCreateInformation,
            .pViewportState = &viewportStateCreateInformation,
            .pRasterizationState = &rasterizationStateCreateInformation,
            .pMultisampleState = &multisampleStateCreateInformation,
            .pDepthStencilState = NULL,
            .pColorBlendState = &colorBlendStateCreateInformation,
            .pDynamicState = &dynamicStateCreateInformation,
            .layout = vulkanContext->pipelineLayout,
            .renderPass = vulkanContext->renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };
        
        HANDLE_VULKAN_RESULT(vkCreateGraphicsPipelines(vulkanContext->device,VK_NULL_HANDLE,1,&graphicsPipelineCreateInformation,NULL,&vulkanContext->graphicsPipeline),"Failed to create Vulkan graphics pipeline",__LINE__,1);
    }
    
    // Create Vulkan swapchain
    if (createSwapchain(vulkanContext,imageSize)) {
        return 1;
    }
    
    // Create Vulkan command pools and command buffers
    {
        VkCommandPoolCreateInfo commandPoolCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = vulkanContext->graphicsQueueFamilyIndex
        };
        
        HANDLE_VULKAN_RESULT(vkCreateCommandPool(vulkanContext->device,&commandPoolCreateInformation,NULL,&vulkanContext->commandPool),"Failed to create Vulkan command pool",__LINE__,1);
        
        vulkanContext->commandBuffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
        
        for (uint8_t commandBufferIndex = 0; commandBufferIndex < MAX_FRAMES_IN_FLIGHT; commandBufferIndex++) {
            VkCommandBufferAllocateInfo commandBufferAllocateInformation = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = vulkanContext->commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
            
            HANDLE_VULKAN_RESULT(vkAllocateCommandBuffers(vulkanContext->device,&commandBufferAllocateInformation,&vulkanContext->commandBuffers[commandBufferIndex]),"Failed to allocate Vulkan command buffer",__LINE__,1);
        }
    }
    
    // Create Vulkan synchronization objects
    {
        vulkanContext->imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
        vulkanContext->renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
        vulkanContext->inFlightFences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
        
        VkSemaphoreCreateInfo semaphoreCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        
        VkFenceCreateInfo fenceCreateInformation = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        
        for (uint8_t synchronizationObjectIndex = 0; synchronizationObjectIndex < vulkanContext->swapchainImageCount; synchronizationObjectIndex++) {
            HANDLE_VULKAN_RESULT(vkCreateSemaphore(vulkanContext->device,&semaphoreCreateInformation,NULL,&vulkanContext->imageAvailableSemaphores[synchronizationObjectIndex]),"Failed to create Vulkan image available semaphore",__LINE__,1);
            
            HANDLE_VULKAN_RESULT(vkCreateSemaphore(vulkanContext->device,&semaphoreCreateInformation,NULL,&vulkanContext->renderFinishedSemaphores[synchronizationObjectIndex]),"Failed to create Vulkan render finished semaphore",__LINE__,1);
            
            HANDLE_VULKAN_RESULT(vkCreateFence(vulkanContext->device,&fenceCreateInformation,NULL,&vulkanContext->inFlightFences[synchronizationObjectIndex]),"Failed to create Vulkan in flight fence",__LINE__,1);
        }
    }
    
    return 0;
}

void deinitializeVulkanContext(struct vulkanContext *vulkanContext) {
    vkDeviceWaitIdle(vulkanContext->device);
    
    for (uint32_t swapchainImageIndex = 0; swapchainImageIndex < vulkanContext->swapchainImageCount; swapchainImageIndex++) {
        vkDestroyFence(vulkanContext->device,vulkanContext->inFlightFences[swapchainImageIndex],NULL);
        vkDestroySemaphore(vulkanContext->device,vulkanContext->renderFinishedSemaphores[swapchainImageIndex],NULL);
        vkDestroySemaphore(vulkanContext->device,vulkanContext->imageAvailableSemaphores[swapchainImageIndex],NULL);
        vkDestroyFramebuffer(vulkanContext->device,vulkanContext->swapchainFramebuffers[swapchainImageIndex],NULL);
        vkDestroyImageView(vulkanContext->device,vulkanContext->swapchainImageViews[swapchainImageIndex],NULL);
    }
    
    free(vulkanContext->inFlightFences);
    free(vulkanContext->renderFinishedSemaphores);
    free(vulkanContext->imageAvailableSemaphores);
    free(vulkanContext->swapchainFramebuffers);
    free(vulkanContext->swapchainImageViews);
    free(vulkanContext->swapchainImages);
    
    vkDestroyCommandPool(vulkanContext->device,vulkanContext->commandPool,NULL);
    
    vkDestroyPipeline(vulkanContext->device,vulkanContext->graphicsPipeline,NULL);
    vkDestroyPipelineLayout(vulkanContext->device,vulkanContext->pipelineLayout,NULL);
    vkDestroyRenderPass(vulkanContext->device,vulkanContext->renderPass,NULL);
    
    for (uint8_t shaderModuleIndex = 0; shaderModuleIndex < vulkanContext->shaderModulesCount; shaderModuleIndex++) {
        vkDestroyShaderModule(vulkanContext->device,vulkanContext->shaderModules[shaderModuleIndex],NULL);
    }
    
    free(vulkanContext->shaderModules);
    
    vkDestroySwapchainKHR(vulkanContext->device,vulkanContext->swapchain,NULL);
    
    vkDestroyDevice(vulkanContext->device,NULL);
    
    if (vulkanContext->debugMessenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->instance,"vkDestroyDebugUtilsMessengerEXT");
        destroyDebugMessenger(vulkanContext->instance,vulkanContext->debugMessenger,NULL);
    }
    
    vkDestroySurfaceKHR(vulkanContext->instance,vulkanContext->surface,NULL);
    
    vkDestroyInstance(vulkanContext->instance,NULL);
}

const _Bool renderVulkanContextFrame(struct vulkanContext *vulkanContext) {
    uint32_t currentFrame = vulkanContext->currentFrame;
    
    HANDLE_VULKAN_RESULT(vkWaitForFences(vulkanContext->device,1,&vulkanContext->inFlightFences[currentFrame],VK_TRUE,UINT64_MAX),"Failed to wait for in Vulkan flight fence",__LINE__,1);
    
    uint32_t imageIndex = 0;
    
    HANDLE_VULKAN_RESULT(vkAcquireNextImageKHR(vulkanContext->device,vulkanContext->swapchain,UINT64_MAX,vulkanContext->imageAvailableSemaphores[currentFrame],VK_NULL_HANDLE,&imageIndex),"Failed to aqquire next Vulkan image",__LINE__,1);
    
    HANDLE_VULKAN_RESULT(vkResetFences(vulkanContext->device,1,&vulkanContext->inFlightFences[currentFrame]),"Failed to reset Vulkan fence",__LINE__,1);
    
    HANDLE_VULKAN_RESULT(vkResetCommandBuffer(vulkanContext->commandBuffers[currentFrame],0),"Failed to reset Vulkan command buffer",__LINE__,1);
    
    // Record command buffer
    {
        VkCommandBufferBeginInfo commandBufferBeginInformation = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0,
            .pInheritanceInfo = NULL
        };
        
        HANDLE_VULKAN_RESULT(vkBeginCommandBuffer(vulkanContext->commandBuffers[currentFrame],&commandBufferBeginInformation),"Failed to begin Vulkan command buffer",__LINE__,1);
        
        VkClearValue clearColor = {{{0,0,0,1}}};
        
        VkRenderPassBeginInfo renderPassBeginInformation = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = vulkanContext->renderPass,
            .framebuffer = vulkanContext->swapchainFramebuffers[imageIndex],
            .renderArea.extent = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent,
            .clearValueCount = 1,
            .pClearValues = &clearColor,
        };
        
        vkCmdBeginRenderPass(vulkanContext->commandBuffers[currentFrame],&renderPassBeginInformation,VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(vulkanContext->commandBuffers[currentFrame],VK_PIPELINE_BIND_POINT_GRAPHICS,vulkanContext->graphicsPipeline);
        
        vulkanContext->viewport.width = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.width;
        vulkanContext->viewport.height = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent.height;
        vkCmdSetViewport(vulkanContext->commandBuffers[currentFrame],0,1,&vulkanContext->viewport);
        
        vulkanContext->scissor.extent = vulkanContext->physicalDeviceSurfaceCapabilities.currentExtent;
        vkCmdSetScissor(vulkanContext->commandBuffers[currentFrame],0,1,&vulkanContext->scissor);
        
        vkCmdDraw(vulkanContext->commandBuffers[currentFrame],3,1,0,0);
        
        vkCmdEndRenderPass(vulkanContext->commandBuffers[currentFrame]);
        
        HANDLE_VULKAN_RESULT(vkEndCommandBuffer(vulkanContext->commandBuffers[currentFrame]),"Failed to end command buffer",__LINE__,1);
    }
    
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    
    VkSubmitInfo submitInformation = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkanContext->imageAvailableSemaphores[currentFrame],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &vulkanContext->commandBuffers[currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vulkanContext->renderFinishedSemaphores[imageIndex]
    };
    
    HANDLE_VULKAN_RESULT(vkQueueSubmit(vulkanContext->graphicsQueueFamily,1,&submitInformation,vulkanContext->inFlightFences[currentFrame]),"Failed to submit Vulkan graphics queue",__LINE__,1);
    
    VkPresentInfoKHR presentInformation = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkanContext->renderFinishedSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &vulkanContext->swapchain,
        .pImageIndices = &imageIndex,
        .pResults = NULL
    };
    
    HANDLE_VULKAN_RESULT(vkQueuePresentKHR(vulkanContext->presentationQueueFamily,&presentInformation),"Failed to submit Vulkan frame",__LINE__,1);
    
    vulkanContext->currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
    return 0;
}

#undef HANDLE_VULKAN_RESULT

#endif
