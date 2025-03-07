#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "graphics/buffer.hpp"
#include "graphics/canvas.hpp"

namespace app
{

class mesh
{
private:
public:
    std::vector<graphics::vertex3d> m_mesh;
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
    mesh(graphics::canvas &context, uint32_t width, uint32_t height) :
        m_width(width),
        m_mesh_buffer(context, width * height * sizeof(m_mesh[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        m_index_buffer(context, (width - 1) * (height - 1) * 6 * sizeof(m_indices[0]), VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        m_normal_buffer(context, width* height * sizeof(m_mesh[0]), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
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
        m_mesh[y * m_width + x].pos = value;
    }

    glm::vec4 get_normal(uint32_t x, uint32_t y)
    {
        //uint32_t quad_no = y * (m_width - 1) * x;
        //uint32_t i = y * (m_width) * x

        glm::vec3 v1 = m_mesh[get_index(x, y)].pos;
        glm::vec3 v2 = m_mesh[get_index(x + 1, y)].pos;
        glm::vec3 v3 = m_mesh[get_index(x + 1, y + 1)].pos;

        glm::vec3 a = v1 - v3;
        glm::vec3 b = v2 - v3;
        glm::vec3 c = glm::cross(a, b);

        v1 = m_mesh[get_index(x, y)].pos;
        v2 = m_mesh[get_index(x + 1, y + 1)].pos;
        v3 = m_mesh[get_index(x, y + 1)].pos;

        a = v1 - v3;
        b = v2 - v3;
        glm::vec3 d = glm::cross(a, b);

        auto e = (c + d);
        e = glm::normalize(e);
        glm::vec4 f = glm::vec4(e, 1.0f);

        //m_normals.push_back(f);
        return f;
    }


    void make_normals()
    {
        for (uint32_t x = 0; x < m_width - 1; x++)
        {
            for (uint32_t y = 0; y < m_width - 1; y++)
            {
                m_normals.push_back(get_normal(x, y));
            }
        }

        for (uint32_t x = 1; x < m_width - 2; x++)
        {
            for (uint32_t y = 1; y < m_width - 2; y++) // TODO: height instead of width
            {
                //glm::vec3 a = m_mesh[get_index(x - 1, y)].pos - m_mesh[get_index(x, y)].pos;
                //glm::vec3 b = m_mesh[get_index(x + 1, y)].pos - m_mesh[get_index(x, y)].pos;
                //glm::vec3 c = m_mesh[get_index(x, y - 1)].pos - m_mesh[get_index(x, y)].pos;
                //glm::vec3 d = m_mesh[get_index(x, y + 1)].pos - m_mesh[get_index(x, y)].pos;

                //a = glm::normalize(a);
                //b = glm::normalize(b);
                //c = glm::normalize(c);
                //d = glm::normalize(d);

                glm::vec3 a = get_normal(x, y);
                glm::vec3 b = get_normal(x + 1, y);
                glm::vec3 c = get_normal(x, y + 1);
                glm::vec3 d = get_normal(x + 1, y + 1);

                glm::vec3 e = a + b + c + d;
                e = glm::normalize(e);

                m_mesh[get_index(x, y)].normal = e;
            }
        }

        return;

        for (uint32_t i = 0; i < m_indices.size(); i += 6)
        {
            glm::vec3 v1 = m_mesh[m_indices[i]].pos;
            glm::vec3 v2 = m_mesh[m_indices[i + 1]].pos;
            glm::vec3 v3 = m_mesh[m_indices[i + 2]].pos;

            glm::vec3 a = v1 - v3;
            glm::vec3 b = v2 - v3;
            glm::vec3 c = glm::cross(a, b);

            v1 = m_mesh[m_indices[i + 3]].pos;
            v2 = m_mesh[m_indices[i + 4]].pos;
            v3 = m_mesh[m_indices[i + 5]].pos;

            a = v1 - v3;
            b = v2 - v3;
            glm::vec3 d = glm::cross(a, b);

            auto e = (c + d);
            e = glm::normalize(e);
            glm::vec4 f = glm::vec4(e, 1.0f);
            
            m_normals.push_back(f);
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