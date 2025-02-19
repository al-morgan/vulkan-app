#pragma once

#include <stdexcept>
#include <glm/glm.hpp>

namespace graphics
{

constexpr uint32_t NUM_FRAMES = 2;


struct vertex2d
{
    glm::vec2 pos;
};

struct vertex3d
{
    glm::vec3 pos;
    glm::vec3 normal;
};

struct framebuffer
{
    VkImage image;
    VkImageView view;
    uint32_t index;
};

static void check(VkResult result)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan error!");
    }
}

}