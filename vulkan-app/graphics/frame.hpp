#include "glm/glm.hpp"
#include "graphics/canvas.hpp"
#include "graphics/buffer.hpp"
#include "graphics/graphics.hpp"

namespace graphics
{


struct mvp
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};


class frame
{
private:
    graphics::canvas& m_context;
public:
    graphics::buffer ubuffer;

    frame(graphics::canvas& context) :
        m_context(context),
        ubuffer(context, sizeof(mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    {
    }

    frame(const graphics::frame& other) :
        frame(other.m_context)
    {

    }

};

}