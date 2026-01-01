#pragma once

#include <vulkan/vulkan.h>
#include <stdatomic.h>
#include <wayland-client.h>
#include <X11/Xlib.h>
#include <meow_utilities/include/meow_math.h>

struct vulkanContext {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueueFamily;
    uint32_t presentationQueueFamilyIndex;
    VkQueue presentationQueueFamily;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR physicalDeviceSurfaceCapabilities;
    VkSurfaceFormatKHR *physicalDeviceSurfaceFormats;
    uint32_t physicalDeviceSurfaceFormatCount;
    VkPresentModeKHR *physicalDeviceSurfacePresentationModes;
    uint32_t physicalDeviceSurfacePresentationModeCount;
    VkSurfaceFormatKHR physicalDeviceSurfaceFormat;
    VkPresentModeKHR physicalDeviceSurfacePresentationMode;
    VkSwapchainKHR swapchain;
    VkImage *swapchainImages;
    uint32_t swapchainImageCount;
    VkImageView *swapchainImageViews;
    VkShaderModule *shaderModules;
    uint8_t shaderModulesCount;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkFramebuffer *swapchainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
    uint8_t currentFrame;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
};

const _Bool createSwapchain(struct vulkanContext *vulkanContext,meow_vector2 imageSize);

const _Bool initializeVulkanContext(struct vulkanContext *vulkanContext,struct wl_surface *surface,struct wl_display *display,Window *x11Window,meow_vector2 imageSize);

void deinitializeVulkanContext(struct vulkanContext *vulkanContext);

const _Bool renderVulkanContextFrame(struct vulkanContext *vulkanContext);
