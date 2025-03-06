#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"

namespace graphics
{

class program_builder
{
public:
    program_builder(graphics::canvas& canvas);
    ~program_builder();
    void add_stage(VkShaderStageFlagBits stage, std::string code);
private:
    graphics::canvas& m_canvas;
    std::vector<VkShaderModule> m_shader_modules;
};

}