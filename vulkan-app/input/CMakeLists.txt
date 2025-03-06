add_library(
    vulkan-app-input
    keyboard.cpp
    mouse.cpp
    )

add_library(
    vulkan-app::input
    ALIAS
    vulkan-app-input
)
