add_custom_target(
    shaders
    COMMAND echo "Compiling shaders!"
) 

function(add_shader shader_name)
    add_custom_command(
        OUTPUT
        ${shader_name}.spv
        COMMAND
        glslc -o ${shader_name}.spv ${CMAKE_CURRENT_LIST_DIR}/${shader_name}
    )

    add_custom_target(
        ${shader_name}
        DEPENDS
        ${shader_name}.spv
    )

    add_dependencies(
        shaders
        ${shader_name}
    )

endfunction()

add_shader(simple.vert)
add_shader(simple.frag)
