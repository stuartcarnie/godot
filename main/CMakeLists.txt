set(GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/register_platform_apis.gen.cpp)
set(GODOT_PLATFORMS ios visionos android web)
generate_godot_register_platform_apis("${GODOT_PLATFORMS}" "${GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT}")

set(GODOT_MAIN_EXECUTABLE_APP_ICON_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/app_icon.png)
set(GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/main/app_icon.gen.h)
generate_file(make_app_icon ${GODOT_MAIN_EXECUTABLE_APP_ICON_SOURCE} ${GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT})

set(GODOT_MAIN_EXECUTABLE_APP_SPLASH_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/splash.png)
set(GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/main/splash.gen.h)
generate_file(make_app_splash ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_SOURCE} ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT})

set(GODOT_EXTENSION_INTERFACE_DUMP_OUTPUT ${CMAKE_BINARY_DIR}/include/core/extension/gdextension_interface_dump.gen.h)
generate_file(gdextension_interface_dumper "${CMAKE_CURRENT_SOURCE_DIR}/../core/extension/gdextension_interface.json" "${GODOT_EXTENSION_INTERFACE_DUMP_OUTPUT}")

set(GODOT_EXTENSION_INTERFACE_HEADER_OUTPUT ${CMAKE_BINARY_DIR}/include/core/extension/gdextension_interface.gen.h)
generate_file(gdextension_interface_header "${CMAKE_CURRENT_SOURCE_DIR}/../core/extension/gdextension_interface.json" "${GODOT_EXTENSION_INTERFACE_HEADER_OUTPUT}")

add_custom_target(generate_main_app_files DEPENDS
        ${GODOT_MAIN_EXECUTABLE_APP_SPLASH_OUTPUT}
        ${GODOT_EXTENSION_INTERFACE_DUMP_OUTPUT}
        ${GODOT_EXTENSION_INTERFACE_HEADER_OUTPUT}
)
set_target_properties(generate_main_app_files PROPERTIES FOLDER "generated")

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
        ../misc/dist/macos_tools.app/Contents/Resources/Assets.car
        ../misc/dist/macos_tools.app/Contents/Resources/GDScript.icns
        ../misc/dist/macos_tools.app/Contents/Resources/GodotLG.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Project.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Resource.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Scene.icns
        ../misc/dist/macos_tools.app/Contents/Resources/Shader.icns
)
# [[[end]]]

set(_godot_target_sources
        ${GODOT_MAIN_EXECUTABLE_APP_ICON_OUTPUT}
        ${GODOT_PLATFORM_GENERATED_REGISTERED_APIS_OUTPUT}
        ${GODOT_EXTENSION_INTERFACE_DUMP_OUTPUT}
        ${GODOT_EXTENSION_INTERFACE_HEADER_OUTPUT}
)

if (LIBRARY_TYPE STREQUAL "executable")
    add_executable(Godot MACOSX_BUNDLE
            ${_godot_target_sources}
            ${MACOS_RESOURCES}
    )
else ()
    if (LIBRARY_TYPE STREQUAL "static_library")
        add_library(Godot STATIC ${_godot_target_sources})
    else ()
        add_library(Godot SHARED ${_godot_target_sources})
    endif ()

    set_target_properties(Godot PROPERTIES
            OUTPUT_NAME godot
            XCODE_ATTRIBUTE_PRODUCT_NAME godot
    )
endif ()

add_dependencies(Godot generate_main_app_files)

if (LIBRARY_TYPE STREQUAL "executable")
    set_target_properties(Godot PROPERTIES RESOURCE "${MACOS_RESOURCES}")
endif ()

if (APPLE)
    get_target_property(SOURCES Godot SOURCES)
    foreach (source IN LISTS SOURCES)
        if ("${source}" MATCHES "\\.(inl|h)$")
            set_source_files_properties("${source}" PROPERTIES XCODE_EXPLICIT_FILE_TYPE sourcecode.cpp.h)
        endif ()
        if ("${source}" MATCHES "\\.icns$")
            set_source_files_properties("${source}" PROPERTIES XCODE_EXPLICIT_FILE_TYPE image.icns)
        endif ()

    endforeach ()
endif ()

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
        steam_tracker.cpp
        steam_tracker.h
)
# [[[end]]]

target_link_libraries(Godot PUBLIC ${GODOT_LIBRARIES})
# target_link_options(Godot PUBLIC -all_load)
target_include_directories(Godot PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})
target_compile_definitions(Godot
        PUBLIC
        NO_EDITOR_SPLASH=1
        PRIVATE
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1>
        $<$<BOOL:${WANT_TESTS}>:TESTS_ENABLED=1>
        OVERRIDE_ENABLED
        OVERRIDE_PATH_ENABLED
)

target_include_directories(Godot PUBLIC
        ${GODOT_ENGINE_ROOT_DIRECTORY}/modules
        ${GODOT_ENGINE_ROOT_DIRECTORY}/editor
        ${GODOT_ENGINE_ROOT_DIRECTORY}/platform
)

if (LIBRARY_TYPE STREQUAL "executable" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(Godot PROPERTIES
            XCODE_ATTRIBUTE_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/misc/dist/macos_tools.app/Contents/Info.plist
            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_SOURCE_DIR}/misc/dist/macos/editor_debug.entitlements
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APPLE_BUNDLE_IDENTIFIER}
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/dist/macos_tools.app/Contents/Info.plist
    )
elseif (LIBRARY_TYPE STREQUAL "executable" AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(_info_plist "${CMAKE_BINARY_DIR}/Info.plist")
    file(WRITE "${_info_plist}" [=[<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleDisplayName</key>
	<string>$(INFOPLIST_KEY_CFBundleDisplayName)</string>
	<key>CFBundleExecutable</key>
	<string>$(EXECUTABLE_NAME)</string>
	<key>CFBundleIcons</key>
	<dict/>
	<key>CFBundleIcons~ipad</key>
	<dict/>
	<key>CFBundleIdentifier</key>
	<string>$(PRODUCT_BUNDLE_IDENTIFIER)</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>$(PRODUCT_NAME)</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleSignature</key>
	<string>GODO</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
	<key>ITSAppUsesNonExemptEncryption</key>
	<false/>
	<key>LSRequiresIPhoneOS</key>
	<true/>
	<key>LSSupportsOpeningDocumentsInPlace</key>
	<true/>
	<key>UIFileSharingEnabled</key>
	<false/>
	<key>UIRequiredDeviceCapabilities</key>
	<array>
		<string>arm64</string>
	</array>
	<key>NSCameraUsageDescription</key>
	<string></string>
	<key>NSPhotoLibraryUsageDescription</key>
	<string></string>
	<key>NSMicrophoneUsageDescription</key>
	<string></string>
	<key>UIRequiresFullScreen</key>
	<true/>
	<key>UIStatusBarHidden</key>
	<true/>
	<key>UISupportedInterfaceOrientations</key>
	<array>
		<string>UIInterfaceOrientationLandscapeLeft</string>
		<string>UIInterfaceOrientationLandscapeRight</string>
		<string>UIInterfaceOrientationPortrait</string>
		<string>UIInterfaceOrientationPortraitUpsideDown</string>
	</array>
	<key>UISupportedInterfaceOrientations~ipad</key>
	<array>
		<string>UIInterfaceOrientationLandscapeLeft</string>
		<string>UIInterfaceOrientationLandscapeRight</string>
		<string>UIInterfaceOrientationPortrait</string>
		<string>UIInterfaceOrientationPortraitUpsideDown</string>
	</array>
	<key>CADisableMinimumFrameDurationOnPhone</key><true/>
	<key>UIApplicationSceneManifest</key>
	<dict>
		<key>UIApplicationSupportsMultipleScenes</key><false/>
		<key>UISceneConfigurations</key>
		<dict>
			<key>UIWindowSceneSessionRoleApplication</key>
			<array>
				<dict>
					<key>UISceneConfigurationName</key>
					<string>Default Configuration</string>
				</dict>
			</array>
		</dict>
	</dict>
</dict>
</plist>
]=])
    set_target_properties(Godot PROPERTIES
            XCODE_ATTRIBUTE_INFOPLIST_FILE "${_info_plist}"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APPLE_BUNDLE_IDENTIFIER}
            MACOSX_BUNDLE_INFO_PLIST "${_info_plist}"
            XCODE_EMBED_FRAMEWORKS "${MOLTENVK_FOUND}"
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY TRUE
    )
elseif (LIBRARY_TYPE STREQUAL "executable" AND CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    # Compute scene manifest based on VISIONOS_APP_ROLE / VISIONOS_IMMERSION_STYLE.
    if (VISIONOS_APP_ROLE STREQUAL "immersive")
        if (VISIONOS_IMMERSION_STYLE STREQUAL "full")
            set(_style "UIImmersionStyleFull")
        else ()
            set(_style "UIImmersionStyleMixed")
        endif ()
        set(_scene_manifest [=[	<key>UIApplicationSceneManifest</key>
	<dict>
		<key>UIApplicationPreferredDefaultSceneSessionRole</key>
		<string>CPSceneSessionRoleImmersiveSpaceApplication</string>
		<key>UIApplicationSupportsMultipleScenes</key>
		<true/>
		<key>UISceneConfigurations</key>
		<dict>
			<key>UISceneSessionRoleImmersiveSpaceApplication</key>
			<array>
				<dict>
					<key>UISceneInitialImmersionStyle</key>
					<string>@_IMMERSION_STYLE@</string>
				</dict>
			</array>
		</dict>
	</dict>]=])
        string(REPLACE "@_IMMERSION_STYLE@" "${_style}" _scene_manifest "${_scene_manifest}")
    else ()
        set(_scene_manifest [=[	<key>UIApplicationSceneManifest</key>
	<dict>
		<key>UIApplicationSupportsMultipleScenes</key><false/>
		<key>UISceneConfigurations</key>
		<dict>
			<key>UIWindowSceneSessionRoleApplication</key>
			<array>
				<dict>
					<key>UISceneConfigurationName</key>
					<string>Default Configuration</string>
				</dict>
			</array>
		</dict>
	</dict>]=])
    endif ()

    set(_info_plist "${CMAKE_BINARY_DIR}/Info.plist")
    string(CONFIGURE [=[<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CADisableMinimumFrameDurationOnPhone</key>
	<true/>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleDisplayName</key>
	<string>$(INFOPLIST_KEY_CFBundleDisplayName)</string>
	<key>CFBundleExecutable</key>
	<string>$(EXECUTABLE_NAME)</string>
	<key>CFBundleIcons</key>
	<dict/>
	<key>CFBundleIcons~ipad</key>
	<dict/>
	<key>CFBundleIdentifier</key>
	<string>$(PRODUCT_BUNDLE_IDENTIFIER)</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>$(PRODUCT_NAME)</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0.0</string>
	<key>CFBundleSupportedPlatforms</key>
	<array>
		<string>XROS</string>
	</array>
	<key>CFBundleSignature</key>
	<string>GODO</string>
	<key>CFBundleVersion</key>
	<string>1.0.0</string>
	<key>ITSAppUsesNonExemptEncryption</key>
	<false/>
	<key>LSRequiresIPhoneOS</key>
	<true/>
	<key>LSSupportsOpeningDocumentsInPlace</key>
	<false/>
	<key>MinimumOSVersion</key>
	<string>26.0</string>
	<key>NSCameraUsageDescription</key>
	<string></string>
	<key>NSMicrophoneUsageDescription</key>
	<string></string>
	<key>NSPhotoLibraryUsageDescription</key>
	<string></string>
	<key>UIDeviceFamily</key>
	<array>
		<integer>7</integer>
	</array>
	<key>UIFileSharingEnabled</key>
	<false/>
	<key>UILaunchStoryboardName</key>
	<string>Launch Screen</string>
	<key>UIRequiredDeviceCapabilities</key>
	<array/>
	<key>UIRequiresFullScreen</key>
	<true/>
	<key>UIStatusBarHidden</key>
	<true/>
	<key>UISupportedInterfaceOrientations</key>
	<array>
		<string>UIInterfaceOrientationLandscapeLeft</string>
	</array>
	<key>UISupportedInterfaceOrientations~ipad</key>
	<array>
		<string>UIInterfaceOrientationLandscapeRight</string>
	</array>
@_scene_manifest@
</dict>
</plist>
]=] _plist @ONLY)
    file(WRITE "${_info_plist}" "${_plist}")

    set_target_properties(Godot PROPERTIES
            XCODE_ATTRIBUTE_INFOPLIST_FILE "${_info_plist}"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APPLE_BUNDLE_IDENTIFIER}
            MACOSX_BUNDLE_INFO_PLIST "${_info_plist}"
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY TRUE
    )
elseif (LIBRARY_TYPE STREQUAL "executable" AND CMAKE_SYSTEM_NAME STREQUAL "tvOS")
    set(_info_plist "${CMAKE_BINARY_DIR}/Info.plist")
    file(WRITE "${_info_plist}" [=[<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleDisplayName</key>
	<string>$(INFOPLIST_KEY_CFBundleDisplayName)</string>
	<key>CFBundleExecutable</key>
	<string>$(EXECUTABLE_NAME)</string>
	<key>CFBundleIdentifier</key>
	<string>$(PRODUCT_BUNDLE_IDENTIFIER)</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>$(PRODUCT_NAME)</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0.0</string>
	<key>CFBundleSupportedPlatforms</key>
	<array>
		<string>AppleTVOS</string>
	</array>
	<key>CFBundleSignature</key>
	<string>GODO</string>
	<key>CFBundleVersion</key>
	<string>1.0.0</string>
	<key>ITSAppUsesNonExemptEncryption</key>
	<false/>
	<key>LSRequiresIPhoneOS</key>
	<true/>
	<key>MinimumOSVersion</key>
	<string>26.0</string>
	<key>UIDeviceFamily</key>
	<array>
		<integer>3</integer>
	</array>
	<key>UIRequiredDeviceCapabilities</key>
	<array>
		<string>arm64</string>
	</array>
	<key>UIRequiresFullScreen</key>
	<true/>
	<key>UIApplicationSceneManifest</key>
	<dict>
		<key>UIApplicationSupportsMultipleScenes</key><false/>
		<key>UISceneConfigurations</key>
		<dict>
			<key>UIWindowSceneSessionRoleApplication</key>
			<array>
				<dict>
					<key>UISceneConfigurationName</key>
					<string>Default Configuration</string>
				</dict>
			</array>
		</dict>
	</dict>
</dict>
</plist>
]=])
    set_target_properties(Godot PROPERTIES
            XCODE_ATTRIBUTE_INFOPLIST_FILE "${_info_plist}"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APPLE_BUNDLE_IDENTIFIER}
            MACOSX_BUNDLE_INFO_PLIST "${_info_plist}"
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY TRUE
            XCODE_GENERATE_SCHEME TRUE
            XCODE_SCHEME_LLDB_INIT_FILE "${CMAKE_SOURCE_DIR}/.lldbinit"
    )
endif ()

if (LIBRARY_TYPE STREQUAL "executable")
    if (GODOT_PCK_FILE)
        cmake_path(GET GODOT_PCK_FILE STEM _product_name)
    else ()
        set(_product_name "Godot")
    endif ()
    set_target_properties(Godot PROPERTIES
            XCODE_ATTRIBUTE_PRODUCT_NAME "${_product_name}"
    )

    if (GODOT_PCK_FILE AND CMAKE_SYSTEM_NAME MATCHES "^(iOS|tvOS|visionOS)$")
        target_sources(Godot PRIVATE "${GODOT_PCK_FILE}")
        set_source_files_properties("${GODOT_PCK_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif ()
endif ()
