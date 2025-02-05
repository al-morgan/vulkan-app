#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>


#include "gfx.hpp"
#include <vector>
#include <limits>
#include <optional>
#include <fstream>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "file.hpp"
#include "vk.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "graphics/graphics.hpp"
#include "graphics/descriptor_set.hpp"
#include "graphics/context.hpp"
#include "graphics/buffer.hpp"
#include "graphics/swapchain.hpp"
#include "graphics/image.hpp"
#include "graphics/pass.hpp"

#include "perlin.hpp"

#define WIDTH	800
#define HEIGHT	800


static bool jumping;


struct mvp
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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

void app::engine::update(graphics::context& context, app::window& window, vk::command_buffer& command_buffer)
{
    std::srand(std::time(nullptr));

    app::perlin noise(10, 10);

    graphics::buffer vbuffer(context, 112 * 12 * 3 * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    graphics::buffer ubuffer(context, sizeof(mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    constexpr int axis_size = 1000;

    static graphics::vertex3d mesh[axis_size + 1][axis_size + 1];

    for (int x = 0; x <= axis_size; x++)
    {
        for (int y = 0; y <= axis_size; y++)
        {
            mesh[y][x].pos[0] = static_cast<float>(x) / static_cast<float>(axis_size);
            mesh[y][x].pos[1] = static_cast<float>(y) / static_cast<float>(axis_size);
            mesh[y][x].pos[2] = noise.get(mesh[y][x].pos[0], mesh[y][x].pos[1]) * 0.06f;
        }
    }

    //std::vector<graphics::vertex3d> normals;
    std::vector<glm::vec4> normals;

    std::vector<graphics::vertex3d> points;

    for (int x = 0; x < axis_size; x++)
    {
        for (int y = 0; y < axis_size; y++)
        {
            points.push_back(mesh[y][x]);
            points.push_back(mesh[y][x + 1]);
            points.push_back(mesh[y + 1][x]);

            points.push_back(mesh[y][x + 1]);
            points.push_back(mesh[y + 1][x + 1]);
            points.push_back(mesh[y + 1][x]);

            glm::vec3 a = mesh[y][x].pos - mesh[y][x + 1].pos;
            glm::vec3 b = mesh[y + 1][x].pos - mesh[y][x + 1].pos;
            glm::vec3 c = glm::cross(a, b);
            c = glm::normalize(c);
            glm::vec4 d(c.x, c.y, c.z, 1.0f);
            normals.push_back(d);

            //normals.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));


            a = mesh[y][x + 1].pos - mesh[y + 1][x + 1].pos;
            b = mesh[y + 1][x].pos - mesh[y + 1][x + 1].pos;
            c = glm::cross(a, b);
            c = glm::normalize(c);
            d = glm::vec4(c.x, c.y, c.z, 1.0f);
            normals.push_back(d);

            //normals.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        }
    }

    graphics::swapchain swapchain(context, WIDTH, HEIGHT, context.surface);

    graphics::vertex3d bar = mesh[900][900];

    graphics::buffer new_vertex_buffer(context, points.size() * sizeof(graphics::vertex3d), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    memcpy(new_vertex_buffer.data(), points.data(), points.size() * sizeof(graphics::vertex3d));

    graphics::image depth_buffer(context, WIDTH, HEIGHT, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, false);

    glm::vec3 position(.20f, .20f, .20f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    double fall_speed = 0.0;

    graphics::buffer rbuffer(context, 128 * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    float* mem = static_cast<float*>(rbuffer.data());
    for (uint32_t i = 0; i < 100; i += 1)
    {
        mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);
    }

    graphics::buffer nbuffer(context, sizeof(normals[0]) * normals.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    glm::vec3* nmem = static_cast<glm::vec3*>(nbuffer.data());
    memcpy(nmem, normals.data(), sizeof(normals[0]) * normals.size());

    graphics::pass my_pass(context);
    my_pass.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
    my_pass.add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    my_pass.add_binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    my_pass.commit();
    my_pass.finalize(context.vertex_shader, context.fragment_shader);
    my_pass.update(0, rbuffer);
    my_pass.update(1, ubuffer);
    my_pass.update(2, nbuffer);

    auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());

    double frame_count = 0;

    while (!glfwWindowShouldClose(window.glfw_window))
    {
        auto elapsed = std::chrono::steady_clock::now();
        const std::chrono::duration<double> diff = elapsed - start;
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(elapsed);        
        auto blah = now_ms - start;
        std::cout << frame_count / blah.count() * 1000.0 << std::endl;
        frame_count++;
        
        glfwPollEvents();
        vkWaitForFences(context.device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetCommandBuffer(command_buffer, 0);
        vkResetFences(context.device, 1, &m_in_flight_fence);
        mvp* ubo = static_cast<mvp*>(ubuffer.data());

        direction[0] = sin(input::get_yaw());
        direction[1] = cos(input::get_yaw());
        direction[2] = -sin(input::get_pitch());

        glm::vec3 left(sin(input::get_yaw() - 1.57), cos(input::get_yaw() - 1.57), -sin(input::get_pitch()));

        double floor = noise.get(position[0], position[1]) * 0.06f + 0.04f;

        if (position[2] < floor)
        {
            position[2] = floor;
            jumping = false;
        }
        else
        {
            fall_speed += .0001f;
        }

        if (input::is_pressed(input::KEY_FORWARD))
        {
            position += direction * 0.001f;
        }

        if (input::is_pressed(input::KEY_LEFT))
        {
            position += left * 0.001f;
        }

        if (input::is_pressed(input::KEY_BACKWARD))
        {
            position -= direction * 0.001f;
        }

        if (input::is_pressed(input::KEY_RIGHT))
        {
            position -= left * 0.001f;
        }

        position[2] -= fall_speed;

        ubo->view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 0.0f, 1.0f));

        ubo->model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));

        ubo->model = glm::mat4(1);/*  glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));*/

        ubo->proj = glm::perspective(glm::radians(45.0f),
            800.0f / 800.0f, 0.001f,
            10.0f);

        ubo->proj[1][1] *= -1.0f;

        swapchain.get_next_framebuffer();

        context.begin_command_buffer(command_buffer);

        if (frame_count == 1)
        {
            vbuffer.copy(command_buffer);
            rbuffer.copy(command_buffer);
            nbuffer.copy(command_buffer);
            new_vertex_buffer.copy(command_buffer);
        }

        ubuffer.copy(command_buffer);

        depth_buffer.transition(command_buffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_NONE, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, my_pass.m_pipeline_layout, 0, 1, &my_pass.m_descriptor_set, 0, nullptr);
        swapchain.prepare_swapchain_for_writing(command_buffer);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, my_pass.m_pipeline);

        VkDeviceSize offset = 0;
        VkBuffer buffers[] = { new_vertex_buffer.handle() };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, &offset);

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

        context.begin_rendering(command_buffer, swapchain.image_view(), depth_buffer.view());
        vkCmdDraw(command_buffer, 6000000, 1, 0, 0);
        vkCmdEndRendering(command_buffer);

        swapchain.prepare_swapchain_for_presentation(command_buffer);

        vkEndCommandBuffer(command_buffer);

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        context.submit(
            command_buffer,
            swapchain.get_semaphore(),
            wait_stage,
            m_render_finished_semaphore,
            m_in_flight_fence);

        swapchain.present(m_render_finished_semaphore);

        // NO NO NO
        //vkDeviceWaitIdle(context.device);
    }

}
//}
