

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vulkan/vulkan.h>

#include "vk.hpp"
#include "gfx.hpp"
#include "graphics/context.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#define WIDTH 800
#define HEIGHT 800

int main()
{
	app::window window;
	graphics::context context(window.handle, WIDTH, HEIGHT);

	// This stuff is per thread
	vk::command_pool command_pool(context.device, context.graphics_queue.family_index);
	vk::command_buffer command_buffer(context.device, command_pool);

    input::init_keyboard(window.glfw_window);
    input::init_mouse(window.glfw_window);

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
