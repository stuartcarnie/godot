godot_add_library(godot-server-rendering-renderer_rd-effects FOLDER rendering STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-renderer_rd-effects PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-renderer_rd-effects PRIVATE
        bokeh_dof.cpp
        bokeh_dof.h
        copy_effects.cpp
        copy_effects.h
        debug_effects.cpp
        debug_effects.h
        fsr.cpp
        fsr.h
        fsr2.cpp
        fsr2.h
        luminance.cpp
        luminance.h
        resolve.cpp
        resolve.h
        roughness_limiter.cpp
        roughness_limiter.h
        sort_effects.cpp
        sort_effects.h
        ss_effects.cpp
        ss_effects.h
        taa.cpp
        taa.h
        tone_mapper.cpp
        tone_mapper.h
        vrs.cpp
        vrs.h
)
# [[[end]]]

target_link_libraries(godot-server-rendering-renderer_rd-effects PRIVATE godot-config)
