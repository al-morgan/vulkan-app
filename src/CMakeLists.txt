add_subdirectory(graphics)
add_subdirectory(input)

add_executable(
    vulkan-app
    file.cpp
    gfx.cpp
    main.cpp
    mesh.cpp
    perlin.cpp
    window.cpp
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
    shaders 
)
