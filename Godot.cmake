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

    set(no_values NO_INCLUDES)
    set(single_values FOLDER)
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
        set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER ${_ARG_FOLDER})
    endif ()

    if (NOT _ARG_NO_INCLUDES)
        target_include_directories(${LIBRARY_NAME} PRIVATE
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_BINARY_DIR}/include # ensures generated files can be found
        )
    endif ()
    set(GODOT_LIBRARIES ${GODOT_LIBRARIES} ${LIBRARY_NAME} CACHE STRING "All the godot library targets" FORCE)
endfunction()

function(group_sources)
    foreach(f ${ARGN})
        get_filename_component(abs_path ${f} REALPATH )
        get_filename_component(filename ${f} NAME )
        string (REPLACE ${filename} "" abs_path ${abs_path})
        source_group("" FILES ${f})
        if(abs_path)
            #strip of the cmake source dir
            string(REPLACE ${CMAKE_CURRENT_BINARY_DIR} "Generated Files" rel_path ${abs_path}) #in case of generated files
            string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" rel_path ${rel_path})
            if (rel_path)
                string(REPLACE ${CMAKE_SOURCE_DIR} "" rel_path ${rel_path})
            endif()
            if (rel_path)
                string(REPLACE "/" "\\" group ${rel_path})
                source_group(${group} FILES ${f})
            endif(rel_path)
        endif(abs_path)
    endforeach(f)
endfunction(group_sources)


# Groups all sources of the specified target according to the dir tree
# this will affect especially the VS Filters
function(group_target_sources target)
    get_target_property(s ${target} SOURCES)
    group_sources(${s})
endfunction(group_target_sources)
