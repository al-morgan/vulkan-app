#include <functional>
#include <vector>


#include "vulkan/vulkan.h"

#include "graphics/buffer.hpp"
#include "graphics/image.hpp"
#include "graphics/context.hpp"


namespace graphics
{
	class descriptor_set
	{
	private:
		graphics::context& m_context;
		VkDescriptorSetLayout m_layout;
		std::vector<VkDescriptorSetLayoutBinding> m_bindings;

	public:
		descriptor_set(graphics::context& context) : m_context(context) { }

		void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
		{
			VkDescriptorSetLayoutBinding dsl_binding{};
			dsl_binding.binding = binding;
			dsl_binding.descriptorType = descriptor_type;
			dsl_binding.stageFlags = stage_flags;
			m_bindings.push_back(dsl_binding);
		}

		void commit()
		{
			VkDescriptorSetLayoutCreateInfo layout_create_info{};
			layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout_create_info.bindingCount = m_bindings.size();
			layout_create_info.pBindings = m_bindings.data();
			vkCreateDescriptorSetLayout(m_context.device, &layout_create_info, nullptr, &m_layout);
		}
	};
}