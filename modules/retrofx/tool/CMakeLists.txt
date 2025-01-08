add_executable(slang-tool)
target_compile_features(slang-tool PRIVATE cxx_std_20)
target_compile_definitions(slang-tool PRIVATE UNIX_ENABLED)
set_target_properties(slang-tool PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20)
set_target_properties(slang-tool PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC YES)
target_sources(slang-tool PRIVATE
        main.cpp
        dummy.cpp
        ../../../platform/macos/dir_access_macos.h
        ../../../platform/macos/dir_access_macos.mm
)
target_link_libraries(slang-tool PUBLIC
        driver-unix
        godot-config
        godot-core
        godot-crypto
        godot-debugger
        godot-error
        godot-extension
        godot-input
        godot-io
        godot-math
        godot-object
        godot-os
        godot-string
        godot-templates
        godot-variant
        godot-module-slang
        godot-module-regex
        $<LINK_LIBRARY:FRAMEWORK,Foundation.framework>
)
