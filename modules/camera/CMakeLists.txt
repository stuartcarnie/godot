godot_add_library(godot-module-camera FOLDER modules STATIC)
target_sources(godot-module-camera PRIVATE
        camera_macos.h
        camera_macos.mm
        #camera_win.cpp
        #camera_win.h
        register_types.cpp
        register_types.h
)

target_link_libraries(godot-module-camera PUBLIC godot-object)
