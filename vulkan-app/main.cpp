

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vulkan/vulkan.h>

#include "gfx.hpp"

int main()
{
    app::gfx gfx;

    try
    {
        gfx.update();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}