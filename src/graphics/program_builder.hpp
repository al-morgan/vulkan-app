#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"
#include "graphics/program.hpp"

namespace graphics
{

class program_builder
{
public:
    program_builder(graphics::canvas& canvas);
    ~program_builder();
    void add_stage(VkShaderStageFlagBits stage, std::string code);
    void reset_stages();
    void add_set(uint32_t set);
    void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
    graphics::program get_program();
    VkDescriptorSet get_descriptor_set();
    VkDescriptorPool                                m_descriptor_pool;
private:
    graphics::canvas&                               m_canvas;
    std::vector<VkShaderModule>                     m_shader_modules;
    std::vector<VkPipelineShaderStageCreateInfo>    m_pipeline_shader_stage_cis;
    //std::vector<VkDescriptorSetLayoutBinding>       m_bindings;
    std::vector<VkDescriptorSetLayout>              m_set_layouts;
    std::vector<VkPipelineLayout>                   m_pipeline_layouts;
    std::vector<VkPipeline>                         m_pipelines;
    uint32_t                                        m_current_set;
    bool                                            m_set_dirty;

    std::vector<std::vector<VkDescriptorSetLayoutBinding>> m_bindings;

    //VkDescriptorSetLayout                       m_layout = VK_NULL_HANDLE;

};

}