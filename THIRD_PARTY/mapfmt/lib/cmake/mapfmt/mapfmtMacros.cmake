# mapfmt_add_map(<target> INPUTS <a.map> [<b.map> ...] [OUTPUT_DIR <dir>])
#
# Compiles .map files to .mapb with mapc as part of your build.
# Creates an ALL target <target>; the outputs are <OUTPUT_DIR>/<name>.mapb
# (OUTPUT_DIR defaults to the current binary dir) and are also listed in the
# MAPFMT_OUTPUTS target property.
function(mapfmt_add_map target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "OUTPUT_DIR" "INPUTS")

    if(NOT TARGET mapfmt::mapc)
        message(FATAL_ERROR "mapfmt_add_map: target mapfmt::mapc not available "
                            "(configure with MAPFMT_BUILD_TOOL=ON or install mapfmt with the tool)")
    endif()
    if(NOT ARG_INPUTS)
        message(FATAL_ERROR "mapfmt_add_map: INPUTS is required")
    endif()
    if(NOT ARG_OUTPUT_DIR)
        set(ARG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    set(outputs)
    foreach(in IN LISTS ARG_INPUTS)
        cmake_path(ABSOLUTE_PATH in BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                   NORMALIZE OUTPUT_VARIABLE in_abs)
        cmake_path(GET in_abs STEM name)
        set(out "${ARG_OUTPUT_DIR}/${name}.mapb")

        add_custom_command(
            OUTPUT  "${out}"
            COMMAND mapfmt::mapc "${in_abs}" "${out}"
            DEPENDS "${in_abs}"
            COMMENT "mapc: ${name}.map -> ${name}.mapb"
            VERBATIM)
        list(APPEND outputs "${out}")
    endforeach()

    add_custom_target(${target} ALL DEPENDS ${outputs})
    set_property(TARGET ${target} PROPERTY MAPFMT_OUTPUTS "${outputs}")
endfunction()
