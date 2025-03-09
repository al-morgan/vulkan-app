#include <algorithm>
#include <stdexcept>

#include <GLFW/glfw3.h>

#include "input/mouse.hpp"
#include "input/state.hpp"

namespace input
{
static double zoom = 1.0;
static double yaw = 3.14159 / 4;
static double pitch = .2;
static double last_update_x, last_update_y;
static double mouse_x, mouse_y;
static bool is_mouse_down;
static bool is_right_mouse_down;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    input::state* state = static_cast<input::state*>(glfwGetWindowUserPointer(window));
    if (state->show_console)
    {
        return;
    }
    
    constexpr double sensitivity = 0.001;

    yaw += (xpos)*sensitivity;
    pitch += (ypos)*sensitivity;
    pitch = std::clamp(pitch, -3.14159 / 2, 3.14159 / 2);

    glfwSetCursorPos(window, 0.0, 0.0);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double constexpr factor = 1.1;

    if (yoffset > 0)
    {
        zoom *= factor;
    }
    else
    {
        zoom /= factor;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    input::state* state = static_cast<input::state*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        is_mouse_down = true;
        last_update_x = mouse_x;
        last_update_y = mouse_y;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        is_mouse_down = false;
    }

}

void init_mouse(GLFWwindow* window)
{
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwSetCursorPos(window, 0.0, 0.0);
    }
    else
    {
        throw std::runtime_error("Could not set mouse to raw input mode!");
    }
}

double get_yaw()
{
    return yaw;
}

double get_pitch()
{
    return pitch;
}

}