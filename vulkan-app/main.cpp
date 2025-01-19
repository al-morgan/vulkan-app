

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vulkan/vulkan.h>

#include "vk.hpp"
#include "gfx.hpp"

#define WIDTH 800
#define HEIGHT 800

int main()
{

	app::window window;
	vk::instance instance;
	vk::physical_device physical_device(instance);
	vk::surface surface(instance, window.handle);
	vk::device device(physical_device, surface);
	vk::queue present_queue(device, device.queue_family_index);
	vk::queue graphics_queue(device, device.queue_family_index);
	vk::swapchain swapchain(device, surface, WIDTH, HEIGHT);

	vk::command_pool command_pool(device, device.queue_family_index);
	vk::command_buffer command_buffer(device, command_pool);
	vk::shader_module fragment_shader_module(device, "./shaders/fragment/simple.spv");
	vk::shader_module vertex_shader_module(device, "./shaders/vertex/simple.spv");
	vk::descriptor_pool descriptor_pool(device);
	vk::descriptor_set_layout layout(device);
	vk::descriptor_set descriptor_set(device, descriptor_pool, layout);
	vk::pipeline_layout pipeline_layout(device, layout);
	vk::pipeline pipeline(device, pipeline_layout, vertex_shader_module, fragment_shader_module, WIDTH, HEIGHT);

	app::gfx gfx(device);

    try
    {
        gfx.update(window, device, command_buffer, descriptor_set, swapchain, pipeline_layout, pipeline, present_queue);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
