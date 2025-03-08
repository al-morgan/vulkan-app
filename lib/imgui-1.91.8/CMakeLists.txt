add_library(
    imgui
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_impl_glfw.cpp
    imgui_impl_vulkan.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

target_include_directories(
    imgui
    PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/lib/glfw-3.4/include
)