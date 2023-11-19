add_executable(metal-tools)
target_sources(metal-tools PRIVATE main.mm)
target_compile_options(metal-tools PRIVATE -fexceptions)
target_link_libraries(metal-tools PUBLIC
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