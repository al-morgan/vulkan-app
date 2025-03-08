add_compile_options("/std:c++20")

add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
add_compile_definitions(GLFW_INCLUDE_VULKAN)
add_compile_definitions(GLFW_EXPOSE_NATIVE_WIN32)

add_subdirectory(graphics)
add_subdirectory(input)

add_executable(
    vulkan-app

    file.cpp
    file.hpp
    gfx.cpp
    gfx.hpp
    main.cpp
    mesh.cpp
    mesh.hpp
    perlin.cpp
    perlin.hpp
    window.cpp
    window.hpp
)

target_link_libraries(
    vulkan-app
    PRIVATE
    vulkan-app::graphics
    vulkan-app::input
    ${Vulkan_LIBRARIES}
)

target_include_directories(
    vulkan-app
    PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/lib/glfw-3.4/include
)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    # target_compile_options(vulkan-app PRIVATE "/W4")
    target_compile_options(vulkan-app PRIVATE "/std:c++20")
endif()

add_dependencies(
    vulkan-app
    shaders_build
)
