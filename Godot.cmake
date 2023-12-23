function(set_abs _out_var _base_directory)
    precondition(_out_var
            MESSAGE "_out_var is required")
    precondition(_base_directory
            MESSAGE "BASE_DIRECTORY is required")

    set(${_out_var} "")
    foreach (X IN ITEMS ${ARGN})
        file(REAL_PATH ${X} X_ABS BASE_DIRECTORY ${_base_directory})
        list(APPEND ${_out_var} ${X_ABS})
    endforeach ()

    return(PROPAGATE ${_out_var})
endfunction()

# These are used to collect all the library targets in a list
set(GODOT_LIBRARIES CACHE STRING "All the godot library targets" FORCE)

# Add a library
# godot_add_library <name>
function(godot_add_library LIBRARY_NAME)
    precondition(LIBRARY_NAME)

    set(no_values "")
    set(single_values FOLDER NO_INCLUDES)
    set(multi_values "")
    cmake_parse_arguments(PARSE_ARGV 1
            _ARG
            "${no_values}"
            "${single_values}"
            "${multi_values}"
    )

    # collect args to forward to add_library
    # Source: Professional CMake: A Practical Guide
    set(quotedArgs "")
    foreach (arg IN LISTS _ARG_UNPARSED_ARGUMENTS)
        string(APPEND quotedArgs " [===[${arg}]===]")
    endforeach ()
    cmake_language(EVAL CODE "add_library(${LIBRARY_NAME} ${quotedArgs})")

    if (DEFINED _ARG_FOLDER)
        set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER Godot/${_ARG_FOLDER})
    endif ()

    if (NOT DEFINED _ARG_NO_INCLUDES)
        target_include_directories(${LIBRARY_NAME} PRIVATE
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_BINARY_DIR}/include # ensures generated files can be found
        )
    endif ()
    set(GODOT_LIBRARIES ${GODOT_LIBRARIES} ${LIBRARY_NAME} CACHE STRING "All the godot library targets" FORCE)
endfunction()