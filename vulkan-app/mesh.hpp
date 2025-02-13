#include <vector>

#include "glm/glm.hpp"

#include "graphics/buffer.hpp"
#include "graphics/context.hpp"

namespace app
{

class mesh
{
private:
public:
    std::vector<glm::vec3> m_mesh;
    std::vector<uint32_t>m_indices;

    graphics::buffer m_mesh_buffer;
    graphics::buffer m_index_buffer;
    
    int m_width;

    uint32_t get_index(uint32_t x, uint32_t y)
    {
        return y * m_width + x;
    }

    mesh(graphics::context &context, uint32_t width, uint32_t height) :
        m_width(width),
        m_mesh_buffer(context, width * height * sizeof(m_mesh[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        m_index_buffer(context, (width - 1) * (height - 1) * 6 * sizeof(m_indices[0]), VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        m_mesh.resize(width * height);
        m_indices.reserve((width - 1) * (height - 1) * 6);

        for (uint32_t x = 0; x < width - 1; x++)
        {
            for (uint32_t y = 0; y < height - 1; y++)
            {
                m_indices.push_back(get_index(x, y));
                m_indices.push_back(get_index(x + 1, y));
                m_indices.push_back(get_index(x + 1, y + 1));

                m_indices.push_back(get_index(x, y));
                m_indices.push_back(get_index(x + 1, y + 1));
                m_indices.push_back(get_index(x, y + 1));
            }
        }
    }

    void set(int x, int y, glm::vec3 value)
    {
        m_mesh[y * m_width + x] = value;
    }
};

}