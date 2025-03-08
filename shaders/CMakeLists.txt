add_custom_target(
    shaders_build
    COMMAND echo "Compiling shaders!"
    SOURCES
    ""
) 

function(add_shader shader_name)
    add_custom_command(
        OUTPUT
        ${shader_name}.spv
        COMMAND
        glslc -o ${shader_name}.spv ${CMAKE_CURRENT_LIST_DIR}/${shader_name}
        DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/${shader_name}
    )

    add_custom_target(
        ${shader_name}
        DEPENDS
        ${shader_name}.spv
        SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/${shader_name}
    )

    add_dependencies(
        shaders_build
        ${shader_name}
    )

    get_target_property(shaders_sources shaders_build SOURCES)
    if(shaders_sources STREQUAL "shaders_sources-NOTFOUND")
        set(shaders_sources "")
    endif()
#        set_target_properties(shaders PROPERTIES SOURCES "${shaders_sources}")
    list(APPEND shaders_sources ${shader_name})
    set_target_properties(shaders_build PROPERTIES SOURCES "${shaders_sources}")

    set_target_properties(${shader_name} PROPERTIES FOLDER shaders)

endfunction()

add_shader(simple.vert)
add_shader(simple.frag)

add_shader(base.vert)
add_shader(base.frag)
add_shader(normaldebug.geom)
