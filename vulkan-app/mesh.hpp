#pragma once

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
    std::vector<uint32_t> m_indices;
    std::vector<glm::vec4> m_normals;

    graphics::buffer m_mesh_buffer;
    graphics::buffer m_index_buffer;
    graphics::buffer m_normal_buffer;

    int m_width;

    uint32_t get_index(uint32_t x, uint32_t y)
    {
        return y * m_width + x;
    }

    // Should width and height be the number of squares or the
    // number of points? Probably points.
    mesh(graphics::context &context, uint32_t width, uint32_t height) :
        m_width(width),
        m_mesh_buffer(context, width * height * sizeof(m_mesh[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        m_index_buffer(context, (width - 1) * (height - 1) * 6 * sizeof(m_indices[0]), VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        m_normal_buffer(context, (width - 1) * (height - 1) * 2 * sizeof(m_normals[0]), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
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

    void make_normals()
    {
        for (uint32_t i = 0; i < m_indices.size(); i += 6)
        {
            glm::vec3 v1 = m_mesh[m_indices[i]];
            glm::vec3 v2 = m_mesh[m_indices[i + 1]];
            glm::vec3 v3 = m_mesh[m_indices[i + 2]];

            glm::vec3 a = v1 - v3;
            glm::vec3 b = v2 - v3;
            glm::vec3 c = glm::cross(a, b);
            //c = glm::normalize(c);
            //glm::vec4 d = glm::vec4(c, 1.0f);

            v1 = m_mesh[m_indices[i + 3]];
            v2 = m_mesh[m_indices[i + 4]];
            v3 = m_mesh[m_indices[i + 5]];

            a = v1 - v3;
            b = v2 - v3;
            glm::vec3 d = glm::cross(a, b);
            //c = glm::normalize(c);
            //glm::vec4 d = glm::vec4(c, 1.0f);

            auto e = (c + d);
            e = glm::normalize(e);
            glm::vec4 f = glm::vec4(e, 1.0f);
            
            m_normals.push_back(f);
            //m_normals.push_back(f);
        }
    }

    void copy(VkCommandBuffer command_buffer)
    {
        memcpy(m_mesh_buffer.data(), m_mesh.data(), m_mesh.size() * sizeof(m_mesh[0]));
        memcpy(m_index_buffer.data(), m_indices.data(), m_indices.size() * sizeof(m_indices[0]));
        memcpy(m_normal_buffer.data(), m_normals.data(), m_normals.size() * sizeof(m_normals[0]));

        m_mesh_buffer.copy(command_buffer);
        m_index_buffer.copy(command_buffer);
        m_normal_buffer.copy(command_buffer);
    }
};

}