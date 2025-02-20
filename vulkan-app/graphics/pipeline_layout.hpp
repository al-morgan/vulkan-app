#include <vector>

#include "vulkan/vulkan.h"

#include "graphics/context.hpp"

namespace graphics
{

class pipeline_layout
{
public:
    pipeline_layout(graphics::context& context);
    void add_set(uint32_t set);
private:
    graphics::context& m_context;
    VkDescriptorSetLayoutCreateInfo m_descriptor_set_layout_ci;
};

}