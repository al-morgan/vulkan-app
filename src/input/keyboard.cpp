#include <GLFW/glfw3.h>

#include "input/keyboard.hpp"
#include "input/state.hpp"

namespace input
{
static bool status[KEY_COUNT];

static void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool pressed = false;


    input::state* state = static_cast<input::state *>(glfwGetWindowUserPointer(window));

    input::keys which_key;

    if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)
    {
        state->show_console = !state->show_console;

        // console on: regular mouse mode.
        // console off: FPS mouse mode.
        if (state->show_console)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            glfwSetCursorPos(window, 0.0, 0.0);
        }

    }

    switch (action)
    {
    case GLFW_PRESS:
        pressed = true;
        break;
    case GLFW_RELEASE:
        pressed = false;
        break;
    case GLFW_REPEAT:
        return;
    }

    switch (key)
    {
    case GLFW_KEY_W:
        which_key = KEY_FORWARD;
        break;
    case GLFW_KEY_A:
        which_key = KEY_LEFT;
        break;
    case GLFW_KEY_S:
        which_key = KEY_BACKWARD;
        break;
    case GLFW_KEY_D:
        which_key = KEY_RIGHT;
        break;
    case GLFW_KEY_C:
        which_key = KEY_DOWN;
        break;
    case GLFW_KEY_SPACE:
        which_key = KEY_UP;
        break;
    default:
        return;
    }

    status[which_key] = pressed;
}

void init_keyboard(GLFWwindow* window)
{
    glfwSetKeyCallback(window, key_press_callback);
}

bool is_pressed(input::keys key)
{
    return status[key];
}

}