#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "file.hpp"
#include "graphics/canvas.hpp"

namespace graphics
{

class shader_builder
{
public:
    shader_builder(graphics::canvas& m_context);
    ~shader_builder();
    VkShaderModule get_result();
    void set_code(std::string filename);
private:
    graphics::canvas& m_context;
    std::vector<VkShaderModule> m_shaders;
    std::string m_filename;
};

}