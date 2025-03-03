#include <iostream>
#include <vector>
#include <optional>

#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "graphics/graphics.hpp"
#include "graphics/canvas.hpp"

static VkInstance create_instance();
static VkPhysicalDevice get_physical_device(VkInstance instance);
static VkPhysicalDeviceMemoryProperties get_memory_properties(VkPhysicalDevice physical_device);
static uint32_t get_memory_type_index(VkPhysicalDeviceMemoryProperties& properties, VkMemoryPropertyFlags flags);
static VkSurfaceKHR create_surface(VkInstance instance, HWND window_handle);
static std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice physical_device);
static VkDevice create_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<VkQueueFamilyProperties>& queue_family_properties);
static uint32_t get_queue_family_index(VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<VkQueueFamilyProperties>& queue_family_properties, VkQueueFlags flags);

graphics::canvas::canvas(HWND window_handle, uint32_t width, uint32_t height) :
    m_width(width),
    m_height(height),
    m_instance(create_instance()),
    m_physical_device(get_physical_device(m_instance)),
    m_memory_properties(get_memory_properties(m_physical_device)),
    m_memory_type_device_local(get_memory_type_index(m_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)),
    m_memory_type_host_coherent(get_memory_type_index(m_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)),
    m_surface(create_surface(m_instance, window_handle)),
    m_queue_family_properties(get_queue_family_properties(m_physical_device)),
    m_device(create_device(m_physical_device, m_surface, m_queue_family_properties))
{
    vkGetDeviceQueue(m_device, graphics_queue.family_index, 0, &graphics_queue.handle);
    graphics_queue.family_index = get_queue_family_index(m_physical_device, m_surface, m_queue_family_properties, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    create_swapchain();

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_semaphore);

    for (auto& frame : m_frames)
    {
        vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &frame.render_finished_semaphore);
        vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &frame.swapchain_semaphore);

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(m_device, &fence_create_info, nullptr, &frame.in_flight_fence);
    }
}

void graphics::canvas::create_swapchain()
{
    VkExtent2D extent{};
    extent.width = m_width;
    extent.height = m_height;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_surface;
    create_info.minImageCount = 3;
    create_info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;
    vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain);

    uint32_t swapchain_image_count;
    std::vector<VkImage> images;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);
    images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, images.data());

    m_framebuffers.resize(swapchain_image_count);

    for (uint32_t i = 0; i < swapchain_image_count; i++)
    {
        m_framebuffers[i].index = i;
        m_framebuffers[i].image = images[i];

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = m_framebuffers[i].image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.layerCount = 1;
        image_view_create_info.subresourceRange.levelCount = 1;
        vkCreateImageView(m_device, &image_view_create_info, nullptr, &m_framebuffers[i].view);
    }
}

graphics::canvas::~canvas()
{
    vkDeviceWaitIdle(m_device);

    for (auto& frame : m_frames)
    {
        vkDestroySemaphore(m_device, frame.render_finished_semaphore, nullptr);
        vkDestroySemaphore(m_device, frame.swapchain_semaphore, nullptr);
        vkDestroyFence(m_device, frame.in_flight_fence, nullptr);
    }

    for (auto device_memory : allocated_device_memory)
    {
        vkFreeMemory(m_device, device_memory, nullptr);
    }

    vkDestroySemaphore(m_device, m_semaphore, nullptr);

    for (uint32_t i = 0; i < static_cast<uint32_t>(m_framebuffers.size()); i++)
    {
        vkDestroyImageView(m_device, m_framebuffers[i].view, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void graphics::canvas::begin_command_buffer(VkCommandBuffer command_buffer)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(command_buffer, &begin_info);
}

void graphics::canvas::begin_rendering(VkCommandBuffer command_buffer, VkImageView view, VkImageView depth_view)
{
    VkClearValue clear_value{};

    VkRect2D render_area{};
    render_area.extent.width = m_width;
    render_area.extent.height = m_height;

    VkRenderingAttachmentInfo color_attachment_info{};
    color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment_info.clearValue = clear_value;
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    color_attachment_info.imageView = view;
    color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    clear_value.depthStencil.depth = 1.0f;
    VkRenderingAttachmentInfo depth_attachment_info{};
    depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment_info.clearValue = clear_value;
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_info.imageView = depth_view;

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pColorAttachments = &color_attachment_info;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.layerCount = 1;
    rendering_info.renderArea = render_area;
    rendering_info.pDepthAttachment = &depth_attachment_info;

    vkCmdBeginRendering(command_buffer, &rendering_info);
}

void graphics::canvas::transition_image(VkCommandBuffer command_buffer,
    VkImage image,
    VkShaderStageFlags source_stage,
    VkAccessFlags source_access_mask,
    VkShaderStageFlags desintation_stage,
    VkAccessFlags destination_access_mask,
    VkImageLayout old_layout,
    VkImageLayout new_layout) const
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcAccessMask = source_access_mask;
    barrier.dstAccessMask = destination_access_mask;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    vkCmdPipelineBarrier(command_buffer, source_stage, desintation_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void graphics::canvas::submit(VkCommandBuffer command_buffer, VkPipelineStageFlags wait_stage)
{
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.pWaitSemaphores = &m_frames[m_current_frame].swapchain_semaphore;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.pSignalSemaphores = &m_frames[m_current_frame].render_finished_semaphore;
    submit_info.signalSemaphoreCount = 1;
    vkQueueSubmit(graphics_queue.handle, 1, &submit_info, m_frames[m_current_frame].in_flight_fence);
}

void graphics::canvas::upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size)
{
    VkDeviceMemory device_memory;

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = buffer_size;
    memory_allocate_info.memoryTypeIndex = m_memory_type_host_coherent;
    check(vkAllocateMemory(m_device, &memory_allocate_info, nullptr, &device_memory));

    VkBindBufferMemoryInfo bind_buffer_memory_info{};
    bind_buffer_memory_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
    bind_buffer_memory_info.memory = device_memory;
    bind_buffer_memory_info.buffer = buffer;
    check(vkBindBufferMemory(m_device, buffer, device_memory, 0));

    void* mem;
    check(vkMapMemory(m_device, device_memory, 0, buffer_size, 0, &mem));
    memcpy(mem, source, buffer_size);
    vkUnmapMemory(m_device, device_memory);

    allocated_device_memory.push_back(device_memory);
}

void graphics::canvas::get_next_framebuffer()
{
    uint32_t image_index = 0;
    vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_frames[m_current_frame].swapchain_semaphore, nullptr, &image_index);
    m_current_framebuffer = m_framebuffers[image_index];
}

void graphics::canvas::prepare_swapchain_for_writing(VkCommandBuffer command_buffer)
{
    transition_image(
        command_buffer,
        m_current_framebuffer.image,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_ACCESS_NONE,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

void graphics::canvas::prepare_swapchain_for_presentation(VkCommandBuffer command_buffer)
{
    transition_image(
        command_buffer,
        m_current_framebuffer.image,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_NONE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );
}

void graphics::canvas::present()
{
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pSwapchains = &m_swapchain;
    present_info.swapchainCount = 1;
    present_info.pWaitSemaphores = &m_frames[m_current_frame].render_finished_semaphore;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &m_current_framebuffer.index;
    vkQueuePresentKHR(graphics_queue.handle, &present_info);
}

uint32_t graphics::canvas::get_height()
{
    return m_height;
}

uint32_t graphics::canvas::get_width()
{
    return m_width;
}

void graphics::canvas::begin_frame()
{
    vkWaitForFences(m_device, 1, &m_frames[m_current_frame].in_flight_fence, VK_TRUE, UINT64_MAX);
    m_current_frame = (m_current_frame + 1) % m_frames.size();
    vkResetFences(m_device, 1, &m_frames[m_current_frame].in_flight_fence);
}






static VkInstance create_instance()
{
    VkInstance instance;

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulcan App";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Unknown Engine.";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> enabled_extensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };

    uint32_t instance_extension_count = 0;
    std::vector<VkExtensionProperties> instance_extensions;
    graphics::check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
    instance_extensions.resize(instance_extension_count);
    graphics::check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

    uint32_t instance_layer_count;
    std::vector<VkLayerProperties> instance_layers;
    graphics::check(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));
    instance_layers.resize(instance_layer_count);
    graphics::check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

    std::vector<const char*> enabled_layers = { "VK_LAYER_KHRONOS_validation" };

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.ppEnabledExtensionNames = enabled_extensions.data();
    create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
    create_info.ppEnabledLayerNames = enabled_layers.data();

    graphics::check(vkCreateInstance(&create_info, nullptr, &instance));

    return instance;
}

static VkPhysicalDevice get_physical_device(VkInstance instance)
{
    uint32_t                        physical_device_count;
    std::vector<VkPhysicalDevice>   physical_devices;

    graphics::check(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
    physical_devices.resize(physical_device_count);
    graphics::check(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));

    if (physical_device_count != 1)
    {
        throw std::runtime_error("Multiple physical devices not supported!");
    }

    // I only have one physical device right now so I'm going to cheat
    return physical_devices[0];
}

static VkPhysicalDeviceMemoryProperties get_memory_properties(VkPhysicalDevice physical_device)
{
VkPhysicalDeviceMemoryProperties memory_properties{};
vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
return memory_properties;
}

static uint32_t get_memory_type_index(VkPhysicalDeviceMemoryProperties& properties, VkMemoryPropertyFlags flags)
{
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++)
    {
        if (properties.memoryTypes[i].propertyFlags == flags)
        {
            return i;
        }
    }

    throw std::runtime_error("Could not find memory types!");
    return 0;
}

static VkSurfaceKHR create_surface(VkInstance instance, HWND window_handle)
{
    VkSurfaceKHR surface;
    VkWin32SurfaceCreateInfoKHR create_info{};

    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = window_handle;
    create_info.hinstance = GetModuleHandle(nullptr);
    graphics::check(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));

    return surface;
}

static std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice physical_device)
{
    std::vector<VkQueueFamilyProperties> properties;
    uint32_t count;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
    properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, properties.data());

    return properties;
}

static VkDevice create_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<VkQueueFamilyProperties>& queue_family_properties)
{
    float queue_priority = 1.0f;
    uint32_t queue_family_index = get_queue_family_index(physical_device, surface, queue_family_properties, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    VkDevice device;

    VkDeviceQueueCreateInfo device_queue_create_info{};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.queueCount = 1;
    device_queue_create_info.queueFamilyIndex = queue_family_index;
    device_queue_create_info.pQueuePriorities = &queue_priority;

    std::vector<const char*> enabled_extensions = { "VK_KHR_swapchain", "VK_KHR_dynamic_rendering" };

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{};
    dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamic_rendering_features.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceFeatures features{};
    features.geometryShader = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = &dynamic_rendering_features;
    create_info.pQueueCreateInfos = &device_queue_create_info;
    create_info.queueCreateInfoCount = 1;
    create_info.ppEnabledExtensionNames = enabled_extensions.data();
    create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    create_info.pEnabledFeatures = &features;

    graphics::check(vkCreateDevice(physical_device, &create_info, nullptr, &device));

    return device;
}

static uint32_t get_queue_family_index(VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<VkQueueFamilyProperties>& queue_family_properties, VkQueueFlags flags)
{
    for (uint32_t i = 0; i < queue_family_properties.size(); i++)
    {
        constexpr VkFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
        VkBool32 surface_support = VK_FALSE;

        graphics::check(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &surface_support));
        if ((queue_family_properties[i].queueFlags & flags) == flags && surface_support)
        {
            return i;
        }
    }

    throw std::runtime_error("Queue selection failed!");
    return 0;
}
