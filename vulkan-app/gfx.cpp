#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>


#include "gfx.hpp"
#include <limits>
#include <optional>
#include <fstream>
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vk.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "graphics/graphics.hpp"
#include "graphics/context.hpp"
#include "graphics/buffer.hpp"
#include "graphics/swapchain.hpp"
#include "graphics/image.hpp"
#include "graphics/pass.hpp"
#include "graphics/frame.hpp"

#include "mesh.hpp"

#include "perlin.hpp"

#define WIDTH	800
#define HEIGHT	800


static bool jumping;



app::window::window()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);



    handle = glfwGetWin32Window(glfw_window);
}

app::window::~window()
{
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

static void vk_check(VkResult result)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan error!");
    }
}

app::engine::engine(graphics::context& context) : context(context)
{
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(context.device, &fence_create_info, nullptr, &m_in_flight_fence);

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);
}

app::engine::~engine()
{
    vkDeviceWaitIdle(context.device);

    vkDestroySemaphore(context.device, m_render_finished_semaphore, nullptr);
    vkDestroyFence(context.device, m_in_flight_fence, nullptr);
}

void app::engine::update(graphics::context& context, app::window& window)
{
    std::srand(std::time(nullptr));

    app::perlin noise(100, 100, 10000.0f, 10000.0f);
    app::perlin noise_low(10, 10, 10000.0f, 10000.0f);

    graphics::buffer vbuffer(context, 112 * 12 * 3 * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    
    // TODO: I'm not using the ubuffer from the frame so it's getting all weird!
    //graphics::buffer ubuffer(context, sizeof(graphics::mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    constexpr int axis_size = 1000;

    app::mesh mesh(context, axis_size, axis_size);

    for (int x = 0; x < axis_size; x++)
    {
        for (int y = 0; y < axis_size; y++)
        {
            glm::vec3 point;

            point[0] = static_cast<float>(x) * 10;
            point[1] = static_cast<float>(y) * 10;
            point[2] = noise.get(point[0], point[1]) * 10.0f;
            point[2] += noise.get(point[0], point[1]) * 100.0f;

            mesh.set(x, y, point);
        }
    }

    mesh.make_normals();

    graphics::swapchain swapchain(context, WIDTH, HEIGHT, context.surface);
    graphics::image depth_buffer(context, WIDTH, HEIGHT, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, false);

    glm::vec3 position(500.f, 500.f, 100.f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    double fall_speed = 0.0;

    graphics::buffer rbuffer(context, 128 * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    float* mem = static_cast<float*>(rbuffer.data());
    for (uint32_t i = 0; i < 100; i += 1)
    {
        mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);
    }

    std::vector<graphics::frame> frame_set;

    graphics::pass my_pass(context);
    my_pass.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
    my_pass.add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    my_pass.add_binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    my_pass.commit();
    my_pass.finalize(context.vertex_shader, context.fragment_shader);
    
    my_pass.update(0, rbuffer);
    my_pass.update(2, mesh.m_normal_buffer);

    auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());

    double frame_count = 0;

    // I think I need to rule-of-three the frame
    // Probably rule-of-five too.

    for (uint32_t i = 0; i < graphics::NUM_FRAMES; i++)
    {
        frame_set.emplace_back(context);
    }

    uint32_t current_frame = 0;

    while (!glfwWindowShouldClose(window.glfw_window))
    {
        auto elapsed = std::chrono::steady_clock::now();
        const std::chrono::duration<double> diff = elapsed - start;
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(elapsed);
        auto blah = now_ms - start;
        std::cout << frame_count / blah.count() * 1000.0 << std::endl;
        frame_count++;

        current_frame = (current_frame + 1) % graphics::NUM_FRAMES;

        my_pass.update(1, frame_set[current_frame].ubuffer);

        glfwPollEvents();
        vkWaitForFences(context.device, 1, &frame_set[current_frame].m_in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetCommandBuffer(frame_set[current_frame].m_command_buffer, 0);
        vkResetFences(context.device, 1, &frame_set[current_frame].m_in_flight_fence);
        graphics::mvp* ubo = static_cast<graphics::mvp*>(frame_set[current_frame].ubuffer.data());

        direction[0] = sin(input::get_yaw());
        direction[1] = cos(input::get_yaw());
        direction[2] = -sin(input::get_pitch());

        glm::vec3 left(sin(input::get_yaw() - 1.57), cos(input::get_yaw() - 1.57), -sin(input::get_pitch()));

        double floor = noise.get(position[0], position[1]) * 60.f + 4.0f;

        float speed = 0.1f;

        if (position[2] < floor)
        {
            position[2] = floor;
            jumping = false;
        }
        else
        {
            fall_speed += .0001f;
        }

        position[2] = 200.f;

        if (input::is_pressed(input::KEY_FORWARD))
        {
            position += direction * speed;
        }

        if (input::is_pressed(input::KEY_LEFT))
        {
            position += left * speed;
        }

        if (input::is_pressed(input::KEY_BACKWARD))
        {
            position -= direction * speed;
        }

        if (input::is_pressed(input::KEY_RIGHT))
        {
            position -= left * speed;
        }

        //position[2] -= fall_speed;

        if (position[2] < 199.f)
        {
            throw std::runtime_error("WHAT");
        }

        ubo->view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 0.0f, 1.0f));

        ubo->model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));

        ubo->model = glm::mat4(1);/*  glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));*/

        ubo->proj = glm::perspective(glm::radians(45.0f),
            800.0f / 800.0f, 0.1f,
            10000.0f);

        ubo->proj[1][1] *= -1.0f;

        swapchain.get_next_framebuffer(frame_set[current_frame].m_swapchain_semaphore);

        context.begin_command_buffer(frame_set[current_frame].m_command_buffer);

        if (frame_count == 1)
        {
            mesh.copy(frame_set[current_frame].m_command_buffer);
        }

        frame_set[current_frame].ubuffer.copy(frame_set[current_frame].m_command_buffer);

        depth_buffer.transition(frame_set[current_frame].m_command_buffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_NONE, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
        vkCmdBindDescriptorSets(frame_set[current_frame].m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, my_pass.m_pipeline_layout, 0, 1, &my_pass.m_descriptor_set, 0, nullptr);
        swapchain.prepare_swapchain_for_writing(frame_set[current_frame].m_command_buffer);
        vkCmdBindPipeline(frame_set[current_frame].m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, my_pass.m_pipeline);

        VkDeviceSize offset = 0;
        VkBuffer buffers[] = { mesh.m_mesh_buffer.handle() };
        vkCmdBindVertexBuffers(frame_set[current_frame].m_command_buffer, 0, 1, buffers, &offset);

        vkCmdBindIndexBuffer(frame_set[current_frame].m_command_buffer, mesh.m_index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(frame_set[current_frame].m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

        context.begin_rendering(frame_set[current_frame].m_command_buffer, swapchain.image_view(), depth_buffer.view());
        vkCmdDrawIndexed(frame_set[current_frame].m_command_buffer, mesh.m_indices.size(), 1, 0, 0, 0);
        vkCmdEndRendering(frame_set[current_frame].m_command_buffer);

        swapchain.prepare_swapchain_for_presentation(frame_set[current_frame].m_command_buffer);

        vkEndCommandBuffer(frame_set[current_frame].m_command_buffer);

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        context.submit(
            frame_set[current_frame].m_command_buffer,
            frame_set[current_frame].m_swapchain_semaphore,
            wait_stage,
            frame_set[current_frame].m_render_finished_semaphore,
            frame_set[current_frame].m_in_flight_fence);

        swapchain.present(frame_set[current_frame].m_render_finished_semaphore);
    }

    vkDeviceWaitIdle(context.device);
}
