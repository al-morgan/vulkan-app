#pragma once

namespace app
{

class window
{
private:
public:
    window();
    ~window();
    GLFWwindow* glfw_window;
    HWND handle;
};

}