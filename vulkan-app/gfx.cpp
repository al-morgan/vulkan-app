#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>


#include "gfx.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "graphics/graphics.hpp"
#include "graphics/canvas.hpp"
#include "graphics/buffer.hpp"
#include "graphics/image.hpp"
#include "graphics/frame.hpp"
#include "graphics/set_layout_builder.hpp"
#include "graphics/set_builder.hpp"
#include "graphics/pipeline_layout_builder.hpp"
#include "graphics/shader_builder.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/descriptor_set_builder.hpp"
#include "graphics/recorder.hpp"
#include "window.hpp"

#include "mesh.hpp"

#include "perlin.hpp"

static bool jumping;

void updateds(graphics::canvas& m_context, uint32_t binding, VkDescriptorSet descriptor_set, VkDescriptorType descriptor_type, graphics::buffer& buffer)
{
    VkDescriptorBufferInfo descriptor_buffer_info{};
    descriptor_buffer_info.buffer = buffer.handle();
    descriptor_buffer_info.offset = 0;
    descriptor_buffer_info.range = buffer.size();

    VkWriteDescriptorSet write_descriptor_set{};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = descriptor_set;
    write_descriptor_set.dstBinding = binding;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = descriptor_type;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
    vkUpdateDescriptorSets(m_context.m_device, 1, &write_descriptor_set, 0, nullptr);
}

app::engine::engine(graphics::canvas& canvas) : context(canvas)
{
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(canvas.m_device, &fence_create_info, nullptr, &m_in_flight_fence);

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(canvas.m_device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);
}

app::engine::~engine()
{
    vkDeviceWaitIdle(context.m_device);

    vkDestroySemaphore(context.m_device, m_render_finished_semaphore, nullptr);
    vkDestroyFence(context.m_device, m_in_flight_fence, nullptr);
}

void app::engine::update(graphics::canvas& canvas, app::window& window)
{
    //std::srand(std::time(nullptr));

    app::perlin noise(100, 100, 10000.0f, 10000.0f);
    app::perlin noise_low(10, 10, 10000.0f, 10000.0f);

    graphics::buffer vbuffer(canvas, 112 * 12 * 3 * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    constexpr int axis_size = 1000;

    app::mesh mesh(canvas, axis_size, axis_size);

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

    //graphics::swapchain swapchain(canvas, WIDTH, HEIGHT, canvas.m_surface);
    graphics::image depth_buffer(canvas, canvas.get_width(), canvas.get_height(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, false);

    glm::vec3 position(500.f, 500.f, 100.f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    double fall_speed = 0.0;

    graphics::buffer rbuffer(canvas, 128 * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    float* mem = static_cast<float*>(rbuffer.data());
    for (uint32_t i = 0; i < 100; i += 1)
    {
        mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);
    }

    std::vector<graphics::frame> frame_set;


    auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());

    double frame_count = 0;

    for (uint32_t i = 0; i < graphics::NUM_FRAMES; i++)
    {
        frame_set.emplace_back(canvas);
    }

    uint32_t current_frame = 0;

    // Good stuff starts here

    graphics::shader_builder shader_builder(canvas);
    shader_builder.set_code("./shaders/vertex/simple.spv");
    VkShaderModule vertex_shader = shader_builder.get_result();

    shader_builder.set_code("./shaders/fragment/simple.spv");
    VkShaderModule fragment_shader = shader_builder.get_result();

    graphics::set_layout_builder my_builder(canvas);
    my_builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
    my_builder.add_binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayout static_set = my_builder.get_result();

    my_builder.reset();
    my_builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayout dynamic_set = my_builder.get_result();

    graphics::pipeline_layout_builder my_pipeline_layout_builder(canvas);
    my_pipeline_layout_builder.add_set(0, static_set);
    my_pipeline_layout_builder.add_set(1, dynamic_set);
    VkPipelineLayout my_layout = my_pipeline_layout_builder.get_result();

    graphics::pipeline_builder pipeline_builder(canvas);
    pipeline_builder.set_fragment_shader(fragment_shader);
    pipeline_builder.set_vertex_shader(vertex_shader);
    pipeline_builder.set_layout(my_layout);
    VkPipeline pipeline = pipeline_builder.get_result();

    graphics::descriptor_set_builder descriptor_set_builder(canvas);
    descriptor_set_builder.set_layout(static_set);
    VkDescriptorSet descriptor_set = descriptor_set_builder.get_result();

    descriptor_set_builder.set_layout(dynamic_set);
    VkDescriptorSet descriptor_set_2 = descriptor_set_builder.get_result();

    updateds(canvas, 0, descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rbuffer);
    updateds(canvas, 1, descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, mesh.m_normal_buffer);

    graphics::recorder recorder(canvas);

    // Next: add layout builder

    while (!glfwWindowShouldClose(window.glfw_window))
    {
        auto elapsed = std::chrono::steady_clock::now();
        const std::chrono::duration<double> diff = elapsed - start;
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(elapsed);
        auto blah = now_ms - start;
        //std::cout << frame_count / blah.count() * 1000.0 << std::endl;
        frame_count++;

        glfwPollEvents();

        VkCommandBuffer command_buffer;

        canvas.begin_frame();
        recorder.begin_frame();

        command_buffer = recorder.get_command_buffer();

        updateds(canvas, 0, descriptor_set_2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frame_set[current_frame].ubuffer);
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

        if (position[2] < 199.f)
        {
            throw std::runtime_error("WHAT");
        }

        ubo->view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 0.0f, 1.0f));
        ubo->model = glm::mat4(1);
        ubo->proj = glm::perspective(glm::radians(45.0f),
            static_cast<float>(canvas.get_width()) / static_cast<float>(canvas.get_height()), 0.1f,
            10000.0f);
        ubo->proj[1][1] *= -1.0f;

        canvas.get_next_framebuffer();

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        if (frame_count == 1)
        {
            mesh.copy(command_buffer);
        }

        frame_set[current_frame].ubuffer.copy(command_buffer);

        depth_buffer.transition(command_buffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_NONE, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

        std::vector<VkDescriptorSet> bindings = { descriptor_set, descriptor_set_2 };

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, my_layout, 0, bindings.size(), bindings.data(), 0, nullptr);
        canvas.prepare_swapchain_for_writing(command_buffer);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        VkBuffer buffers[] = { mesh.m_mesh_buffer.handle() };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, &offset);

        vkCmdBindIndexBuffer(command_buffer, mesh.m_index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

        canvas.begin_rendering(command_buffer, canvas.image_view(), depth_buffer.view());
        vkCmdDrawIndexed(command_buffer, mesh.m_indices.size(), 1, 0, 0, 0);
        vkCmdEndRendering(command_buffer);

        canvas.prepare_swapchain_for_presentation(command_buffer);

        vkEndCommandBuffer(command_buffer);

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        canvas.submit(command_buffer, wait_stage);
        canvas.present();
    }

    vkDeviceWaitIdle(canvas.m_device);
}
