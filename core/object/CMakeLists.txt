# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        callable_mp.cpp
        callable_mp.h
        class_db.cpp
        class_db.h
        editor_language.h
        gdtype.cpp
        gdtype.h
        message_queue.cpp
        message_queue.h
        method_bind.cpp
        method_bind.h
        method_bind_common.h
        method_info.cpp
        method_info.h
        object.cpp
        object.h
        object_id.h
        property_info.cpp
        property_info.h
        ref_counted.cpp
        ref_counted.h
        script_backtrace.cpp
        script_backtrace.h
        script_instance.cpp
        script_instance.h
        script_language.cpp
        script_language.h
        script_language_extension.cpp
        script_language_extension.h
        undo_redo.cpp
        undo_redo.h
        worker_thread_pool.cpp
        worker_thread_pool.h
)
# [[[end]]]
target_include_directories(core PUBLIC
        ${GODOT_ENGINE_ROOT_DIRECTORY}
        ${CMAKE_BINARY_INCLUDE_DIR})

target_include_directories(core PRIVATE
        ${CMAKE_BINARY_INCLUDE_DIR}/modules
        ${GODOT_ENGINE_ROOT_DIRECTORY}/modules)

target_compile_definitions(core PUBLIC
        $<${WANT_DEBUG_FEATURES}:DEBUG_ENABLED>)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_include_directories(core PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/windows)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_include_directories(core PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/macos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_include_directories(core PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/ios)
elseif (CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    target_include_directories(core PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/visionos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(core PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/linuxbsd)
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()
