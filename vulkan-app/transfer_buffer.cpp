//#include <vulkan/vulkan.h>
//#include "gfx_context.hpp"
//
//namespace transfer
//{
//	class buffer
//	{
//	private:
//		const gfx::context& m_context;
//		VkBuffer m_destination;
//		VkBuffer m_source;
//		VkDeviceSize m_size;
//		VkDeviceMemory m_source_memory;
//		VkDeviceMemory m_destination_memory;
//		void* m_memory;
//	public:
//		buffer(const gfx::context& context, size_t size, VkBufferUsageFlags usage);
//		~buffer();
//
//		void copy(VkCommandBuffer command_buffer);
//		VkBuffer handle();
//		void* data();
//	};
//	{
//
//		VkBuffer transfer::buffer::handle()
//		{
//			return m_destination;
//		}
//
//		void* transfer::buffer::data()
//		{
//			return m_memory;
//		}
//
//		transfer::buffer::buffer(const gfx::context & context, size_t size, VkBufferUsageFlags usage) : m_context(context)
//		{
//			m_size = size;
//
//			// Create host buffer
//			VkBufferCreateInfo create_info{};
//			create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//			create_info.size = size;
//			create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//			create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//			vkCreateBuffer(context.device, &create_info, nullptr, &m_source);
//
//			// Create device buffer
//			create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//			vkCreateBuffer(context.device, &create_info, nullptr, &m_destination);
//
//			// Allocate host memory
//			VkMemoryAllocateInfo memory_allocate_info{};
//			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//			memory_allocate_info.allocationSize = size;
//			memory_allocate_info.memoryTypeIndex = context.memory_type_host_coherent;
//			check(vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &m_source_memory));
//			check(vkBindBufferMemory(context.device, m_source, m_source_memory, 0));
//
//			// Allocate device memory
//			memory_allocate_info.memoryTypeIndex = context.memory_type_device_local;
//			check(vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &m_destination_memory));
//			check(vkBindBufferMemory(context.device, m_destination, m_destination_memory, 0));
//
//			// Map the host memory so we can read it
//			check(vkMapMemory(context.device, m_source_memory, 0, size, 0, &m_memory));
//		}
//
//		transfer::buffer::~buffer()
//		{
//			vkFreeMemory(m_context.device, m_destination_memory, nullptr);
//			vkFreeMemory(m_context.device, m_source_memory, nullptr);
//			vkDestroyBuffer(m_context.device, m_source, nullptr);
//			vkDestroyBuffer(m_context.device, m_destination, nullptr);
//		}
//
//		void transfer::buffer::copy(VkCommandBuffer command_buffer)
//		{
//			VkBufferCopy buffer_copy{};
//			buffer_copy.size = m_size;
//			vkCmdCopyBuffer(command_buffer, m_source, m_destination, 1, &buffer_copy);
//
//			// TODO should probably just have one memory barrier
//			VkMemoryBarrier barrier{};
//			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
//			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//			barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
//
//			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
//		}
//	}
//}
