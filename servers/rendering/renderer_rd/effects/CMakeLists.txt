# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
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
        metal_fx.cpp
        metal_fx.h
        motion_vectors_store.cpp
        motion_vectors_store.h
        resolve.cpp
        resolve.h
        roughness_limiter.cpp
        roughness_limiter.h
        smaa.cpp
        smaa.h
        sort_effects.cpp
        sort_effects.h
        spatial_upscaler.h
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

set_property(
        SOURCE metal_fx.cpp
        APPEND
        PROPERTY COMPILE_DEFINITIONS $<$<BOOL:${WANT_METAL}>:METAL_ENABLED RD_ENABLED>
)
