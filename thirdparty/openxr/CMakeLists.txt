godot_add_library(openxr FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(openxr PRIVATE')
#   for file in list_files(cog, ['include/openxr', 'src'], recursive=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(openxr PRIVATE
        include/openxr/openxr.h
        include/openxr/openxr_loader_negotiation.h
        include/openxr/openxr_platform.h
        include/openxr/openxr_platform_defines.h
        include/openxr/openxr_reflection.h
        include/openxr/openxr_reflection_parent_structs.h
        include/openxr/openxr_reflection_structs.h
        src/common/extra_algorithms.h
        src/common/filesystem_utils.cpp
        src/common/filesystem_utils.hpp
        src/common/hex_and_handles.h
        src/common/object_info.cpp
        src/common/object_info.h
        src/common/platform_utils.hpp
        src/common/stdfs_conditions.h
        src/common/unique_asset.h
        src/common/vulkan_debug_object_namer.hpp
        src/common/xr_dependencies.h
        src/common/xr_linear.h
        src/external/jsoncpp/include/json/allocator.h
        src/external/jsoncpp/include/json/assertions.h
        src/external/jsoncpp/include/json/config.h
        src/external/jsoncpp/include/json/forwards.h
        src/external/jsoncpp/include/json/json.h
        src/external/jsoncpp/include/json/json_features.h
        src/external/jsoncpp/include/json/reader.h
        src/external/jsoncpp/include/json/value.h
        src/external/jsoncpp/include/json/version.h
        src/external/jsoncpp/include/json/writer.h
        src/external/jsoncpp/src/lib_json/json_reader.cpp
        src/external/jsoncpp/src/lib_json/json_tool.h
        src/external/jsoncpp/src/lib_json/json_value.cpp
        src/external/jsoncpp/src/lib_json/json_writer.cpp
        src/loader/android_utilities.cpp
        src/loader/android_utilities.h
        src/loader/api_layer_interface.cpp
        src/loader/api_layer_interface.hpp
        src/loader/exception_handling.hpp
        src/loader/loader_core.cpp
        src/loader/loader_init_data.cpp
        src/loader/loader_init_data.hpp
        src/loader/loader_instance.cpp
        src/loader/loader_instance.hpp
        src/loader/loader_logger.cpp
        src/loader/loader_logger.hpp
        src/loader/loader_logger_recorders.cpp
        src/loader/loader_logger_recorders.hpp
        src/loader/loader_platform.hpp
        src/loader/manifest_file.cpp
        src/loader/manifest_file.hpp
        src/loader/runtime_interface.cpp
        src/loader/runtime_interface.hpp
        src/loader/xr_generated_loader.cpp
        src/loader/xr_generated_loader.hpp
        src/xr_generated_dispatch_table_core.c
        src/xr_generated_dispatch_table_core.h
)
# [[[end]]]
target_include_directories(openxr PUBLIC include)
target_include_directories(openxr PRIVATE
        src
        src/common
        src/external/jsoncpp/include)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(openxr PUBLIC XR_OS_LINUX=1 HAVE_SECURE_GETENV=1)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(openxr PUBLIC XR_OS_WINDOWS=1 XR_USE_PLATFORM_WIN32=1)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(openxr PUBLIC XR_OS_APPLE=1)
endif ()
if (${WANT_X11})
    target_compile_definitions(openxr PUBLIC XR_USE_PLATFORM_XLIB=1)
endif ()

if (${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_link_libraries(openxr PUBLIC ${OPENGL_LIBRARIES})
endif ()
if (${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(openxr PUBLIC vulkan)
endif ()
target_compile_options(openxr PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
target_compile_definitions(openxr PUBLIC
        DISABLE_STD_FILESYSTEM
        "$<$<BOOL:${DISABLE_EXCEPTIONS}>:XRLOADER_DISABLE_EXCEPTION_HANDLING;JSON_USE_EXCEPTION=0>"
)