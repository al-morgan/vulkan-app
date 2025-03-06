#include <string>

#include <vulkan/vulkan.h>

#include "file.hpp"
#include "graphics/canvas.hpp"
#include "graphics/program_builder.hpp"

namespace graphics
{

graphics::program_builder::program_builder(graphics::canvas& canvas) :
    m_canvas(canvas)
{

}

graphics::program_builder::~program_builder()
{
    for (auto shader_module : m_shader_modules)
    {
        vkDestroyShaderModule(m_canvas, shader_module, nullptr);
    }
}

void graphics::program_builder::add_stage(VkShaderStageFlagBits stage, std::string code)
{
    std::vector<char> buffer;
    VkShaderModule shader;

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    buffer = app::read_file(code);
    create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
    create_info.codeSize = buffer.size();
    vkCreateShaderModule(m_canvas, &create_info, nullptr, &shader);

    m_shader_modules.push_back(shader);
}


}
