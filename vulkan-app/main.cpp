

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "gfx.hpp"
#include "graphics/canvas.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"

int main()
{
    app::window window;
    graphics::canvas canvas(window.handle, window.get_width(), window.get_height());

    input::init_keyboard(window.glfw_window);
    input::init_mouse(window.glfw_window);

    app::engine gfx(canvas);

    try
    {
        gfx.update(canvas, window);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
