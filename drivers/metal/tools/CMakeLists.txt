add_executable(metal-tools)
target_compile_features(metal-tools PRIVATE cxx_std_20)
set_target_properties(metal-tools PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20)
target_sources(metal-tools PRIVATE main.mm)
target_compile_options(metal-tools PRIVATE -fexceptions)
target_link_libraries(metal-tools PUBLIC
        godot-templates
        zstd
#        cxxopts
        "-framework Foundation"
#        "-framework Cocoa"
#        "-framework Carbon"
#        "-framework AudioUnit"
#        "-framework CoreAudio"
#        "-framework CoreMIDI"
#        "-framework IOKit"
#        "-framework ForceFeedback"
#        "-framework CoreVideo"
#        "-framework AVFoundation"
#        "-framework CoreMedia"
#        "-framework QuartzCore"
#        "-framework Security"
        "-framework Metal"
#        "-framework MetalKit"
#        "-framework IOSurface"
)