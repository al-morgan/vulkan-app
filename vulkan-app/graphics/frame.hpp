#include "glm/glm.hpp"
#include "graphics/context.hpp"
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
    graphics::context& m_context;
public:
    graphics::buffer ubuffer;
    VkFence m_in_flight_fence;
    VkSemaphore m_render_finished_semaphore;
    VkSemaphore m_swapchain_semaphore;

    vk::command_pool m_command_pool;
    vk::command_buffer m_command_buffer;

    frame(graphics::context& context) :
        m_context(context),
        ubuffer(context, sizeof(mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
        m_command_pool(context.device, context.graphics_queue.family_index),
        m_command_buffer(context.device, m_command_pool)
    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(context.device, &fence_create_info, nullptr, &m_in_flight_fence);

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);
        vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_swapchain_semaphore);
    }

    frame(const graphics::frame& other) :
        frame(other.m_context)
    {

    }

    ~frame()
    {
        vkDestroySemaphore(m_context.device, m_render_finished_semaphore, nullptr);
        vkDestroyFence(m_context.device, m_in_flight_fence, nullptr);
    }
};

}