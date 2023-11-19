function(precondition var)
    cmake_parse_arguments(
            PRECONDITION # prefix
            "NEGATE" # options
            "MESSAGE" # single-value args
            "" # multi-value args
            ${ARGN})

    if (PRECONDITION_NEGATE)
        if (${var})
            if (PRECONDITION_MESSAGE)
                message(FATAL_ERROR "Error! ${PRECONDITION_MESSAGE}")
            else()
                message(FATAL_ERROR "Error! Variable ${var} is true or not empty. The value of ${var} is ${${var}}.")
            endif()
        endif()
    else()
        if (NOT ${var})
            if (PRECONDITION_MESSAGE)
                message(FATAL_ERROR "Error! ${PRECONDITION_MESSAGE}")
            else()
                message(FATAL_ERROR "Error! Variable ${var} is false, empty or not set.")
            endif()
        endif()
    endif()
endfunction()

# Assert is 'NOT ${LHS} ${OP} ${RHS}' is true.
function(precondition_binary_op OP LHS RHS)
    cmake_parse_arguments(
            PRECONDITIONBINOP # prefix
            "NEGATE" # options
            "MESSAGE" # single-value args
            "" # multi-value args
            ${ARGN})

    if (PRECONDITIONBINOP_NEGATE)
        if (${LHS} ${OP} ${RHS})
            if (PRECONDITIONBINOP_MESSAGE)
                message(FATAL_ERROR "Error! ${PRECONDITIONBINOP_MESSAGE}")
            else()
                message(FATAL_ERROR "Error! ${LHS} ${OP} ${RHS} is true!")
            endif()
        endif()
    else()
        if (NOT ${LHS} ${OP} ${RHS})
            if (PRECONDITIONBINOP_MESSAGE)
                message(FATAL_ERROR "Error! ${PRECONDITIONBINOP_MESSAGE}")
            else()
                message(FATAL_ERROR "Error! ${LHS} ${OP} ${RHS} is false!")
            endif()
        endif()
    endif()
endfunction()

function(dump_cmake_vars)
    get_cmake_property(variableNames VARIABLES)
    foreach(variableName ${variableNames})
        if(variableName MATCHES "^SWIFT" OR TRUE)
            message("set(${variableName} \"${${variableName}}\")")
        endif()
    endforeach()
endfunction()
