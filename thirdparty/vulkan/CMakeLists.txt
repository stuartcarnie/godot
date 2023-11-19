if (${WANT_VULKAN} AND ${VULKAN_FOUND})
    godot_add_library(vulkan FOLDER thirdparty STATIC)
    target_sources(vulkan PRIVATE
            include/vulkan/vulkan_ggp.h
            include/vulkan/vulkan_core.h
            include/vulkan/vulkan_ios.h
            include/vulkan/vulkan_win32.h
            include/vulkan/vulkan_xcb.h
            include/vulkan/vulkan_directfb.h
            include/vulkan/vulkan_wayland.h
            include/vulkan/vulkan_android.h
            include/vulkan/vulkan_enums.hpp
            include/vulkan/vulkan_xlib.h
            include/vulkan/vulkan_xlib_xrandr.h
            include/vulkan/vk_layer.h
            include/vulkan/vulkan_static_assertions.hpp
            include/vulkan/vulkan_macos.h
            include/vulkan/vulkan.hpp
            include/vulkan/vulkan.h
            include/vulkan/vulkan_handles.hpp
            include/vulkan/vulkan_beta.h
            include/vulkan/vulkan_structs.hpp
            include/vulkan/vulkan_fuchsia.h
            include/vulkan/vulkan_raii.hpp
            include/vulkan/vk_icd.h
            include/vulkan/vulkan_to_string.hpp
            include/vulkan/vulkan_hash.hpp
            include/vulkan/vulkan_screen.h
            include/vulkan/vulkan_format_traits.hpp
            include/vulkan/vulkan_vi.h
            include/vulkan/vulkan_funcs.hpp
            include/vulkan/vulkan_metal.h
            include/vulkan/vk_platform.h
            include/vk_video/vulkan_video_codecs_common.h
            include/vk_video/vulkan_video_codec_h264std_encode.h
            include/vk_video/vulkan_video_codec_h265std.h
            include/vk_video/vulkan_video_codec_h265std_encode.h
            include/vk_video/vulkan_video_codec_h265std_decode.h
            include/vk_video/vulkan_video_codec_h264std.h
            include/vk_video/vulkan_video_codec_h264std_decode.h
            vk_enum_string_helper.h
            vk_mem_alloc.cpp
            vk_mem_alloc.h
    )
    target_include_directories(vulkan PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR})
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(vulkan PUBLIC VK_USE_PLATFORM_XLIB_KHR=1 USE_VOLK=1 VK_ENABLE_BETA_EXTENSIONS=1 VK_KHR_xlib_surface=1 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 VMA_STATIC_VULKAN_FUNCTIONS=1 VMA_VULKAN_VERSION=1003000)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(vulkan PUBLIC VK_USE_PLATFORM_WIN32_KHR=1 USE_VOLK=1 VK_ENABLE_BETA_EXTENSIONS=1 VK_KHR_win32_surface=1 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 VMA_STATIC_VULKAN_FUNCTIONS=1 VMA_VULKAN_VERSION=1003000)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_definitions(vulkan PUBLIC VK_USE_PLATFORM_MACOS_MVK=1 VMA_VULKAN_VERSION=1001000)
    else ()
        message(ERROR "Vulkan not supported correctly on this platform. Revise CMakeLists.txt for thirdparty/vulkan.")
    endif ()
    if (WANT_VOLK)
        target_link_libraries(vulkan PUBLIC volk)
    endif ()
endif ()
