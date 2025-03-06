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
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${Vulkan_INCLUDE_DIRS}
)

add_library(
    vulkan-app::graphics
    ALIAS
    vulkan-app-graphics
)
