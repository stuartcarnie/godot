# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        profiling.cpp
        profiling.h
)
# [[[end]]]

if (GODOT_CORE_PROFILER_SOURCE_FILES)
        godot_add_library(profiler STATIC)
        target_sources(profiler PRIVATE ${GODOT_CORE_PROFILER_SOURCE_FILES})
        if (GODOT_CORE_PROFILER_DEFINITIONS)
                target_compile_definitions(core PRIVATE ${GODOT_CORE_PROFILER_DEFINITIONS})
        endif ()
        target_include_directories(profiler PUBLIC ${GODOT_CORE_PROFILER_INCLUDE_DIRS})
        target_compile_definitions(profiler
                PUBLIC
                TRACY_ENABLE
                $<$<BOOL:${PROFILER_SAMPLE_CALLSTACK}>:TRACY_CALLSTACK=62>
                $<$<BOOL:${TRACY_ON_DEMAND}>:TRACY_ON_DEMAND>
                $<$<BOOL:${GODOT_PROFILER_TRACK_MEMORY}>:GODOT_PROFILER_TRACK_MEMORY>
        )
        target_compile_options(profiler
                PUBLIC
                $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>
                PRIVATE
                $<$<CONFIG:Debug>:-O2>
        )
        target_link_libraries(core PUBLIC profiler)
endif ()

configure_file(profiling.gen.h.in ${CMAKE_BINARY_INCLUDE_DIR}/core/profiling/profiling.gen.h)

# target_include_directories(core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
