# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        dependency_editor.cpp
        dependency_editor.h
        editor_file_system.cpp
        editor_file_system.h
        editor_paths.cpp
        editor_paths.h
        file_info.cpp
        file_info.h
)
# [[[end]]]
