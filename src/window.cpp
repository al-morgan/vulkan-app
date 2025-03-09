#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "window.hpp"
#include "input/state.hpp"


static void window_focus_callback(GLFWwindow* window, int focused)
{
    input::state* state = static_cast<input::state*>(glfwGetWindowUserPointer(window));
    state->focused = focused;
}

app::window::window(void *user_data)
{
    glfwInit();

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vid_mode = glfwGetVideoMode(monitor);
    m_width = vid_mode->width;
    m_height = vid_mode->height;

    // set to false for fullscreen.
    if (false)
    {
        m_width = 800;
        m_height = 800;
        monitor = nullptr;

    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw_window = glfwCreateWindow(m_width, m_height, "Vulkan", monitor, nullptr);

    glfwSetWindowUserPointer(glfw_window, user_data);
    glfwSetWindowFocusCallback(glfw_window, window_focus_callback);

    handle = glfwGetWin32Window(glfw_window);
}

app::window::~window()
{
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

uint32_t app::window::get_height()
{
    return m_height;
}

uint32_t app::window::get_width()
{
    return m_width;
}
