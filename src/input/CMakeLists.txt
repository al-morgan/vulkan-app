add_library(
    vulkan-app-input
    keyboard.cpp
    mouse.cpp
    )

target_include_directories(
    vulkan-app-input
    PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/lib/glfw-3.4/include
)

target_link_libraries(
    vulkan-app-input
    PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/glfw-3.4/lib-vc2022/glfw3.lib
)

add_library(
    vulkan-app::input
    ALIAS
    vulkan-app-input
)
