# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        doc_tools.cpp
        doc_tools.h
        editor_help.cpp
        editor_help.h
        editor_help_search.cpp
        editor_help_search.h
)
# [[[end]]]
