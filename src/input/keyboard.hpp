namespace input
{
enum keys
{
    KEY_FORWARD,
    KEY_BACKWARD,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_UP,
    KEY_COUNT
};

void init_keyboard(GLFWwindow* window);
bool is_pressed(input::keys key);
}