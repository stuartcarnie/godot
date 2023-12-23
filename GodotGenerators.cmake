###
### CMake script for running code generators
###

function(generate_file _cmd _input _output)
    set(no_values "")
    set(single_values WORKING_DIRECTORY)
    set(multi_values "")
    cmake_parse_arguments(PARSE_ARGV 3
            _ARG
            "${no_values}"
            "${single_values}"
            "${multi_values}"
    )

    if (DEFINED _ARG_WORKING_DIRECTORY)
        add_custom_command(
                OUTPUT ${_output}
                COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
                ARGS --env ${GODOT_ENV_FILE} ${_cmd} --input ${_input} --output ${_output}
                DEPENDS ${_input}
                COMMENT "Generating ${_cmd} from ${_input}"
                WORKING_DIRECTORY ${_ARG_WORKING_DIRECTORY}
                VERBATIM
        )
    else ()
        add_custom_command(
                OUTPUT ${_output}
                COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
                ARGS --env ${GODOT_ENV_FILE} ${_cmd} --input ${_input} --output ${_output}
                DEPENDS ${_input}
                COMMENT "Generating ${_cmd} from ${_input}"
                VERBATIM
        )
    endif ()
endfunction()

function(generate_shader_sources _buildType _input _output)
    foreach (_inputFile _outputFile IN ZIP_LISTS _input _output)
        add_custom_command(
                OUTPUT ${_outputFile}
                COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
                ARGS --env ${GODOT_ENV_FILE} ${_buildType} --input ${_inputFile} --output ${_outputFile}
                DEPENDS ${_inputFile}
                COMMENT "Generating ${_buildType} from ${_inputFile}"
                WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
        )
    endforeach ()
endfunction()

function(generate_core_disabled_classes _disabledClasses _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} disabled_classes --output ${_output}
            COMMENT "Generating disabled_classes from ${_disabledClasses}"
    )
endfunction()

function(generate_core_controller_mappings_sources _gameControllerDB _godotControllerDB _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} controller_mappings --input ${_gameControllerDB} ${_godotControllerDB} --output ${_output}
            DEPENDS ${_gameControllerDB} ${_godotControllerDB}
            COMMENT "Generating controller_mappings from ${_gameControllerDB} and ${_godotControllerDB}"
    )
endfunction()

function(generate_license_file _copyrightFile _licenseFile _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} license_header --input-copyright ${_copyrightFile} --input-license ${_licenseFile} --output ${_output}
            DEPENDS ${_copyrightFile} ${_licenseFile}
            COMMENT "Generating license file to ${_output}"
    )
endfunction()

function(generate_export_icon _platformName _iconType _input _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} generate_export_icon --platform-name ${_platformName} --icon-type ${_iconType} --input ${_input} --output ${_output}
            DEPENDS ${_input}
            COMMENT "Generating ${_platformName} ${_iconType} to ${_output}"
            VERBATIM
    )
endfunction()

function(generate_documentation_compressed _input _output _tempFileOutput)
    list(JOIN _input "\n" JOINED_INPUT)
    file(WRITE ${_tempFileOutput} ${JOINED_INPUT})

    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_documentation_header_compressed --output ${_output} --input ${_tempFileOutput}
            DEPENDS ${_input}
            COMMENT "Generating documentation compressed to  ${_output}"
            VERBATIM
    )
endfunction()

function(generate_editor_icons_header _input _output _tempFileOutput)
    list(JOIN _input "\n" JOINED_INPUT)
    file(WRITE ${_tempFileOutput} ${JOINED_INPUT})

    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_editor_icons_action --output ${_output} --input ${_tempFileOutput}
            DEPENDS ${_input}
            COMMENT "Generating documentation compressed to  ${_output}"
            VERBATIM
    )
endfunction()

function(generate_editor_themes_fonts _input _output _tempFileOutput)
    list(JOIN _input "\n" JOINED_INPUT)
    file(WRITE ${_tempFileOutput} ${JOINED_INPUT})

    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_editor_themes_fonts --output ${_output} --input ${_tempFileOutput}
            DEPENDS ${_input}
            COMMENT "Generating documentation compressed to  ${_output}"
            VERBATIM
    )
endfunction()

function(generate_version_information _output _output2)
    add_custom_command(
            OUTPUT ${_output} ${_output2}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_version_data_headers --output ${_output} --output2 ${_output2}
            COMMENT "Generating version information to ${_output} + ${_output2}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
            VERBATIM
    )
endfunction()


function(generate_script_encryption_header _encryptionKey _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            #TODO add input back in
            ARGS --env ${GODOT_ENV_FILE} make_script_encryption_header --output ${_output}
            COMMENT "Generating script encryption key ${_output}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
            VERBATIM
    )
endfunction()

function(generate_godot_extension_wrappers _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_extension_wrapper --output ${_output}
            COMMENT "Generating gdextension wrappers file to ${_output}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
    )
endfunction()

function(generate_godot_gdscript_virtuals _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_gdscript_virtuals --output ${_output}
            DEPENDS ${_input}
            COMMENT "Generating gdscript virtuals file to ${_output}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
    )
endfunction()

function(generate_godot_register_platform_apis _output)
    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_register_platform_apis --output ${_output}
            COMMENT "Generating platform APIs registration ${_output}"
            VERBATIM
    )
endfunction()

function(generate_godot_editor_platform_exporters _inputPlatforms _output)
    list(JOIN _inputPlatforms " " JOINED_INPUT)

    add_custom_command(
            OUTPUT ${_output}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_editor_platform_exporters --output ${_output} --input ${JOINED_INPUT}
            COMMENT "Generating godot editor platform exporters file to ${_output}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
    )
endfunction()

function(generate_enabled_modules_and_register _inputGodotRootEngineDir _customGodotEngineModulesDirectory _modulesEnabledHeader _registerModuleTypeCPP)
    add_custom_command(
            OUTPUT ${_modulesEnabledHeader} ${_registerModuleTypeCPP}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_modules_enabled_and_types --input ${_inputGodotRootEngineDir} --input2 '${_customGodotEngineModulesDirectory}' --output ${_registerModuleTypeCPP} --output2 ${_modulesEnabledHeader}
            COMMENT "Generating godot editor platform exporters file to ${_output}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
    )
endfunction()

function(generate_document_class_paths _outputGeneratedDocClassPaths)
    add_custom_command(
            OUTPUT ${_outputGeneratedDocClassPaths}
            COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
            ARGS --env ${GODOT_ENV_FILE} make_data_class_path --input "${GODOT_ENGINE_ROOT_DIRECTORY}/" --output ${_outputGeneratedDocClassPaths}
            COMMENT "Generating document data class file to ${_output}"
            WORKING_DIRECTORY ${GODOT_ENGINE_ROOT_DIRECTORY}
    )
endfunction()