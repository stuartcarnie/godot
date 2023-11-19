if(${WANT_VOLK} AND ${WANT_VULKAN} AND ${VULKAN_FOUND})
    godot_add_library(volk FOLDER thirdparty STATIC)
    target_sources(volk PRIVATE
            volk.c
            volk.h
    )
    target_include_directories(volk PUBLIC .)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(volk PUBLIC VK_KHR_video_decode_queue=1 VK_ENABLE_BETA_EXTENSIONS=1 VK_KHR_video_queue=1 VK_KHR_xlib_surface=1)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(volk PUBLIC VK_KHR_video_decode_queue=1 VK_ENABLE_BETA_EXTENSIONS=1 VK_KHR_video_queue=1 VK_KHR_win32_surface=1)
    else()
        message(ERROR "Vulkan not supported correctly on this platform. Revise CMakeLists.txt for thirdparty/vulkan.")
    endif()
    target_link_libraries(volk PUBLIC vulkan)
endif()