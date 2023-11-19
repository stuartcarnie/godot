set(GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/register_platform_apis.gen.cpp)
generate_godot_register_platform_apis("${GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT}")

set(GODOT_MAIN_EXECUTABLE_APP_ICON_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/app_icon.png)
set(GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/main/app_icon.gen.h)
generate_file(make_app_icon ${GODOT_MAIN_EXECUTABLE_APP_ICON_SOURCE} ${GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT})

set(GODOT_MAIN_EXECUTABLE_APP_SPLASH_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/splash.png)
set(GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/main/splash.gen.h)
generate_file(make_app_splash ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_SOURCE} ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT})

set(GODOT_INTERFACE_INTERFACE_DUMP_OUTPUT ${CMAKE_BINARY_DIR}/include/core/extension/gdextension_interface_dump.gen.h)
generate_file(gdextension_interface_dumper "${CMAKE_CURRENT_SOURCE_DIR}/../core/extension/gdextension_interface.h" "${GODOT_INTERFACE_INTERFACE_DUMP_OUTPUT}")

add_executable(GodotEditor MACOSX_BUNDLE
        ${GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT}
        ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT}
        ${GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT}
        ${GODOT_INTERFACE_INTERFACE_DUMP_OUTPUT}
)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(GodotEditor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(GodotEditor PRIVATE
        main.cpp
        main.h
        main_timer_sync.cpp
        main_timer_sync.h
        performance.cpp
        performance.h
)
# [[[end]]]

target_link_libraries(GodotEditor PUBLIC ${GODOT_LIBRARIES})
target_include_directories(GodotEditor PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})
target_compile_definitions(GodotEditor
        PUBLIC
        NO_EDITOR_SPLASH=1
        PRIVATE
        $<$<BOOL:${METAL_ENABLED}>:METAL_ENABLED=1>
)

target_include_directories(GodotEditor PUBLIC
        ${GODOT_ENGINE_ROOT_DIRECTORY}/modules
        ${GODOT_ENGINE_ROOT_DIRECTORY}/editor
        ${GODOT_ENGINE_ROOT_DIRECTORY}/platform
)

set_target_properties(GodotEditor PROPERTIES
        XCODE_ATTRIBUTE_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/misc/dist/macos_tools.app/Contents/Info.plist
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_SOURCE_DIR}/misc/dist/macos/editor.entitlements
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/dist/macos_tools.app/Contents/Info.plist
)