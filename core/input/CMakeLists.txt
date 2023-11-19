set(GODOT_CORE_INPUT_GAME_CONTROLLER_MAPPINGS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/gamecontrollerdb.txt)
set(GODOT_CORE_INPUT_GODOT_CONTROLLER_MAPPINGS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/godotcontrollerdb.txt)
set(GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/input/default_controller_mappings.gen.cpp)
generate_core_controller_mappings_sources(${GODOT_CORE_INPUT_GAME_CONTROLLER_MAPPINGS_SOURCE} ${GODOT_CORE_INPUT_GODOT_CONTROLLER_MAPPINGS_SOURCE} ${GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT})

godot_add_library(godot-input FOLDER core STATIC
        ${GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-input PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-input PRIVATE
        default_controller_mappings.h
        input.cpp
        input.h
        input_enums.h
        input_event.cpp
        input_event.h
        input_map.cpp
        input_map.h
        shortcut.cpp
        shortcut.h
)
# [[[end]]]
target_link_libraries(godot-input PUBLIC godot-object)
