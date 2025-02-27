#include "graphics/shader_builder.hpp"

namespace graphics
{

shader_builder::shader_builder(graphics::context& context) :
    m_context(context)
{

}

shader_builder::~shader_builder()
{
    for (auto shader : m_shaders)
    {
        vkDestroyShaderModule(m_context.device, shader, nullptr);
    }
}

void shader_builder::set_code(std::string filename)
{
    m_filename = filename;
}

VkShaderModule shader_builder::get_result()
{
    std::vector<char> buffer;
    VkShaderModule shader;

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    buffer = app::read_file(m_filename);
    create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
    create_info.codeSize = buffer.size();
    vkCreateShaderModule(m_context.device, &create_info, nullptr, &shader);

    m_shaders.push_back(shader);

    return shader;

}



}