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

target_link_libraries(
    vulkan-app-graphics
    PRIVATE
    ${Vulkan_LIBRARIES}
)

target_include_directories(
    vulkan-app-graphics
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/lib/glfw-3.4/include
    ${PROJECT_SOURCE_DIR}/lib/glm-1.0.1
    ${Vulkan_INCLUDE_DIRS}
)

add_library(
    vulkan-app::graphics
    ALIAS
    vulkan-app-graphics
)
