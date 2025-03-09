#include <tuple>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <chrono>

#include <vulkan/vulkan.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "gfx.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "graphics/graphics.hpp"
#include "graphics/canvas.hpp"
#include "graphics/buffer.hpp"
#include "graphics/image.hpp"
#include "graphics/frame.hpp"
#include "graphics/recorder.hpp"
#include "window.hpp"
#include "graphics/program_builder.hpp"
#include "graphics/program.hpp"

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
            point[2] = noise.get(point[0] * 1, point[1] * 1) * 100.0f;
            mesh.set(x, y, point);
        }
    }

    //std::ofstream myfile;
    //myfile.open("terrain.obj", std::ofstream::out | std::ofstream::trunc);
    //
    //for (uint32_t i = 0; i < mesh.m_mesh.size(); i++)
    //{
    //    myfile << "v " << mesh.m_mesh[i].pos[0] << " " << mesh.m_mesh[i].pos[2] << " " << mesh.m_mesh[i].pos[1] << std::endl;
    //}

    //for (uint32_t i = 0; i < mesh.m_indices.size(); i += 3)
    //{
    //    myfile << "f " << mesh.m_indices[i] + 1 << " " << mesh.m_indices[i + 1] + 1 << " " << mesh.m_indices[i + 2] + 1 << std::endl;
    //}

    //myfile.close();

    mesh.make_normals();

    //graphics::swapchain swapchain(canvas, WIDTH, HEIGHT, canvas.m_surface);                                  graphics::image_usage::depth_stencil         graphics::image_aspct::depth
    graphics::image depth_buffer(canvas, canvas.get_width(), canvas.get_height(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, false);

    glm::vec3 position(500.f, 500.f, 200.f);
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

    graphics::recorder recorder(canvas);

    graphics::program_builder program_builder(canvas);

    program_builder.add_set(0);
    program_builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
    program_builder.add_binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSet descriptor_set = program_builder.get_descriptor_set();

    program_builder.add_set(1);
    program_builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);
    VkDescriptorSet descriptor_set_2 = program_builder.get_descriptor_set();

    program_builder.add_stage(VK_SHADER_STAGE_VERTEX_BIT, "simple.vert");
    program_builder.add_stage(VK_SHADER_STAGE_FRAGMENT_BIT, "simple.frag");
    graphics::program program = program_builder.get_program();

    program_builder.reset_stages();
    program_builder.add_stage(VK_SHADER_STAGE_GEOMETRY_BIT, "normaldebug.geom");
    program_builder.add_stage(VK_SHADER_STAGE_VERTEX_BIT, "base.vert");
    program_builder.add_stage(VK_SHADER_STAGE_FRAGMENT_BIT, "base.frag");
    graphics::program program2 = program_builder.get_program();

    updateds(canvas, 0, descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rbuffer);
    updateds(canvas, 1, descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, mesh.m_normal_buffer);


    // IMGUI

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForVulkan(window.glfw_window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = canvas.m_instance;
    init_info.PhysicalDevice = canvas.m_physical_device;
    init_info.Device = canvas.m_device;
    init_info.QueueFamily = canvas.graphics_queue.family_index;
    init_info.Queue = canvas.graphics_queue.handle;
    init_info.PipelineCache = VK_NULL_HANDLE;
    //init_info.DescriptorPool = program_builder.m_descriptor_pool;
    init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE + 1;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = graphics::check;
    init_info.UseDynamicRendering = true;

    VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
    init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &format;
    init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    ImGui_ImplVulkan_Init(&init_info);

    while (!glfwWindowShouldClose(window.glfw_window))
    {
        auto elapsed = std::chrono::steady_clock::now();
        const std::chrono::duration<double> diff = elapsed - start;
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(elapsed);
        auto blah = now_ms - start;
        //std::cout << frame_count / blah.count() * 1000.0 << std::endl;
        frame_count++;

        glfwPollEvents();


        recorder.begin_frame();
        canvas.begin_frame(recorder);

        //ImGui_ImplVulkan_CreateFontsTexture();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)


        updateds(canvas, 0, descriptor_set_2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frame_set[current_frame].ubuffer);
        graphics::mvp* ubo = static_cast<graphics::mvp*>(frame_set[current_frame].ubuffer.data());

        direction[0] = sin(input::get_yaw());
        direction[1] = cos(input::get_yaw());
        direction[2] = -sin(input::get_pitch());

        glm::vec3 left(sin(input::get_yaw() - 1.57), cos(input::get_yaw() - 1.57), -sin(input::get_pitch()));

        double floor = noise.get(position[0], position[1]) * 60.f + 4.0f;

        float speed = 0.1f;

        //if (position[2] < floor)
        //{
        //    position[2] = floor;
        //    jumping = false;
        //}
        //else
        //{
        //    fall_speed += .0001f;
        //}

        //position[2] = 200.f;

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

        if (input::is_pressed(input::KEY_DOWN))
        {
            position[2] -= speed;
        }

        if (input::is_pressed(input::KEY_UP))
        {
            position[2] += speed;
        }

        ubo->view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 0.0f, 1.0f));
        ubo->model = glm::mat4(1);
        ubo->proj = glm::perspective(glm::radians(45.0f),
            static_cast<float>(canvas.get_width()) / static_cast<float>(canvas.get_height()), 0.1f,
            10000.0f);
        ubo->proj[1][1] *= -1.0f;



        if (frame_count == 1)
        {
            mesh.copy(recorder);
        }

        frame_set[current_frame].ubuffer.copy(recorder);

        depth_buffer.transition(recorder, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_NONE, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

        std::vector<VkDescriptorSet> bindings = { descriptor_set, descriptor_set_2 };

        vkCmdBindDescriptorSets(recorder, VK_PIPELINE_BIND_POINT_GRAPHICS, program, 0, bindings.size(), bindings.data(), 0, nullptr);

        VkDeviceSize offset = 0;
        VkBuffer buffers[] = { mesh.m_mesh_buffer.handle() };
        vkCmdBindVertexBuffers(recorder, 0, 1, buffers, &offset);
        vkCmdBindIndexBuffer(recorder, mesh.m_index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(recorder, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

        recorder.begin_rendering(canvas.get_width(), canvas.get_height(), canvas.image_view(), depth_buffer.view());
        recorder.use_program(program);
        vkCmdDrawIndexed(recorder, mesh.m_indices.size(), 1, 0, 0, 0);

        recorder.use_program(program2);
        vkCmdDrawIndexed(recorder, mesh.m_indices.size(), 1, 0, 0, 0);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), recorder);


        recorder.end_rendering();

        canvas.end_frame();
        recorder.end_frame();

        canvas.submit();
    }

    vkDeviceWaitIdle(canvas.m_device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}
