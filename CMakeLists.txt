cmake_minimum_required(VERSION 3.15)
project(vulkan-app VERSION 0.1.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Vulkan)

add_subdirectory(lib/imgui-1.91.8)
add_subdirectory(shaders)
add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vulkan-app)
