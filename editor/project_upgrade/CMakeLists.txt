# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        project_converter_3_to_4.cpp
        project_converter_3_to_4.h
        project_upgrade_tool.cpp
        project_upgrade_tool.h
        renames_map_3_to_4.cpp
        renames_map_3_to_4.h
)
# [[[end]]]
