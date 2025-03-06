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
)
