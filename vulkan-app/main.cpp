

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vulkan/vulkan.h>

#include "vk.hpp"
#include "gfx.hpp"
#include "gfx_context.hpp"

#define WIDTH 800
#define HEIGHT 800

int main()
{
	app::window window;
	gfx::context context(window.handle, WIDTH, HEIGHT);

	//vk::pipeline_layout pipeline_layout(context.device, context.descriptor_set_layout);
	//vk::shader_module fragment_shader_module(context.device, "./shaders/fragment/simple.spv");
	//vk::shader_module vertex_shader_module(context.device, "./shaders/vertex/simple.spv");
	//vk::pipeline pipeline(context.device, WIDTH, HEIGHT);

	// This stuff is per thread
	vk::command_pool command_pool(context.device, context.graphics_queue.family_index);
	vk::command_buffer command_buffer(context.device, command_pool);


	app::engine gfx(context);

    try
    {
		gfx.update(context, window, command_buffer);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
