godot_add_library(godot-editor-project_manager FOLDER export)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-project_manager PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-project_manager PRIVATE
        project_dialog.cpp
        project_dialog.h
        project_list.cpp
        project_list.h
        project_tag.cpp
        project_tag.h
        quick_settings_dialog.cpp
        quick_settings_dialog.h
)
# [[[end]]]
target_link_libraries(godot-editor-project_manager PRIVATE godot-config)
