#pragma once

namespace app
{

class window
{
public:
    window(void *user_data);
    ~window();
    GLFWwindow* glfw_window;
    HWND handle;

    uint32_t get_width();
    uint32_t get_height();

private:
    uint32_t m_width;
    uint32_t m_height;

};

}