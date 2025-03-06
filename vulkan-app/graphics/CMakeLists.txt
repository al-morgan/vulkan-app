add_library(
    vulkan-app-graphics
    buffer.cpp
    canvas.cpp
    frame.cpp
    image.cpp
    program.cpp
    program_builder.cpp
    recorder.cpp
    swapchain.cpp
    )

add_library(
    vulkan-app::graphics
    ALIAS
    vulkan-app-graphics
)
