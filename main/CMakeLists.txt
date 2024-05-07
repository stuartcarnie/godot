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

# macOS specific

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set(MACOS_RESOURCES')
#   for file in list_files(cog, '../misc/dist/macos_tools.app/Contents/Resources', all_files=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set(MACOS_RESOURCES
        ../misc/dist/macos_tools.app/Contents/Resources/GDScript.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Godot.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Project.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Resource.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Scene.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Shader.icns
)
# [[[end]]]

add_executable(Godot MACOSX_BUNDLE
        ${GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT}
        ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT}
        ${GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT}
        ${GODOT_INTERFACE_INTERFACE_DUMP_OUTPUT}
        ${MACOS_RESOURCES}
)

set_target_properties(Godot PROPERTIES RESOURCE "${MACOS_RESOURCES}")

if(APPLE)
    get_target_property(SOURCES Godot SOURCES)
    foreach(source IN LISTS SOURCES)
        if("${source}" MATCHES "\\.(inl|h)$")
            set_source_files_properties("${source}" PROPERTIES XCODE_EXPLICIT_FILE_TYPE sourcecode.cpp.h)
        endif()
        if("${source}" MATCHES "\\.icns$")
            set_source_files_properties("${source}" PROPERTIES XCODE_EXPLICIT_FILE_TYPE image.icns)
        endif()

    endforeach()
endif()

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(Godot PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(Godot PRIVATE
        main.cpp
        main.h
        main_timer_sync.cpp
        main_timer_sync.h
        performance.cpp
        performance.h
)
# [[[end]]]

target_link_libraries(Godot PUBLIC ${GODOT_LIBRARIES})
target_include_directories(Godot PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})
target_compile_definitions(Godot
        PUBLIC
        NO_EDITOR_SPLASH=1
        PRIVATE
        $<$<BOOL:${ENABLE_METAL}>:METAL_ENABLED=1>
)

target_include_directories(Godot PUBLIC
        ${GODOT_ENGINE_ROOT_DIRECTORY}/modules
        ${GODOT_ENGINE_ROOT_DIRECTORY}/editor
        ${GODOT_ENGINE_ROOT_DIRECTORY}/platform
)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
set_target_properties(Godot PROPERTIES
        XCODE_ATTRIBUTE_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/misc/dist/macos_tools.app/Contents/Info.plist
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_SOURCE_DIR}/misc/dist/macos/editor.entitlements
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APPLE_BUNDLE_IDENTIFIER}
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/dist/macos_tools.app/Contents/Info.plist
)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set_target_properties(Godot PROPERTIES
            XCODE_ATTRIBUTE_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/misc/dist/ios/Info.plist
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APPLE_BUNDLE_IDENTIFIER}
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/dist/ios/Info.plist
    )
endif()
