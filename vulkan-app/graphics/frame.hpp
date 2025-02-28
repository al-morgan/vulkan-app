#include "glm/glm.hpp"
#include "graphics/canvas.hpp"
#include "graphics/buffer.hpp"
#include "graphics/graphics.hpp"

namespace graphics
{


struct mvp
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};


class frame
{
private:
    graphics::canvas& m_context;
public:
    graphics::buffer ubuffer;
    VkFence m_in_flight_fence;
    VkSemaphore m_render_finished_semaphore;
    VkSemaphore m_swapchain_semaphore;

    VkCommandPool m_command_pool;
    VkCommandBuffer m_command_buffer;

    frame(graphics::canvas& context) :
        m_context(context),
        ubuffer(context, sizeof(mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(context.device, &fence_create_info, nullptr, &m_in_flight_fence);

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);
        vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_swapchain_semaphore);


        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = context.graphics_queue.family_index;

        check(vkCreateCommandPool(m_context.device, &create_info, nullptr, &m_command_pool));

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandBufferCount = 1;
        alloc_info.commandPool = m_command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        vkAllocateCommandBuffers(m_context.device, &alloc_info, &m_command_buffer);
    }

    frame(const graphics::frame& other) :
        frame(other.m_context)
    {

    }

    ~frame()
    {
        vkDestroySemaphore(m_context.device, m_render_finished_semaphore, nullptr);
        vkDestroySemaphore(m_context.device, m_swapchain_semaphore, nullptr);
        vkDestroyFence(m_context.device, m_in_flight_fence, nullptr);
    }
};

}