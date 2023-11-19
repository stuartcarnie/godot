cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(GodotUtils)

project(godot
        LANGUAGES C CXX OBJC OBJCXX
)

###
# Apple Specific
###

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "6N425KH923")

include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#########
# OPTIONS
#########

# Editor build
set(WANT_EDITOR 1)

# Components
set(PRECISION "single" CACHE STRING "Set the floating-point precision level")
set_property(CACHE PRECISION PROPERTY STRINGS single double)
option(ENABLE_MINIZIP "Enable ZIP archive support using minizip" ON)
option(ENABLE_BROTLI "Enable Brotli for decompresson and WOFF2 fonts support" ON)
option(DISABLE_EXCEPTIONS "Force disabling exception handling code" ON)
option(ENABLE_DEV_BUILD "Enable developer build" OFF)
option(ENABLE_DEBUG_FEATURES "Enable debug features" ON)

# Third party libraries
cmake_dependent_option(BUILTIN_BROTLI "Use the built-in Brotli library" ON "ENABLE_BROTLI" OFF)

#########
# Configure variables and build from options
#########

if (ENABLE_BROTLI AND NOT BUILTIN_BROTLI)
    find_package(Brotli REQUIRED)
endif ()

if (ENABLE_DEV_BUILD)
    message(STATUS "Enabling developer build settings")
    add_compile_definitions(DEV_ENABLED)
endif ()

if (PRECISION STREQUAL "single")
    set(WANT_DOUBLE_PRECISION 0)
elseif (PRECISION STREQUAL "double")
    set(WANT_DOUBLE_PRECISION 1)
    message(STATUS "Use double precision")
else ()
    message(FATAL_ERROR "Invalid PRECISION: ${PRECISION}. Expect single or double")
endif ()

if (ENABLE_DEBUG_FEATURES)
    message(STATUS "Enabling debug features")
    set(WANT_DEBUG_FEATURES 1)
endif ()

add_compile_definitions($<${WANT_DOUBLE_PRECISION}:REAL_T_IS_DOUBLE>)

add_compile_options(
        $<$<AND:$<BOOL:${DISABLE_EXCEPTIONS}>,$<COMPILE_LANGUAGE:CXX>>:-fno-exceptions>

        # We often implement `operator<` for structs of pointers as a requirement
        # for putting them in `Set` or `Map`. We don't mind about unreliable ordering.
        "$<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wshadow-field-in-constructor;-Wshadow-uncaptured-local;-Wno-ordered-compare-function-pointers;-Wno-tautological-compare>"
)

####################################
#Step 1: Get options and set base directories
####################################

option(GODOT_EXTERNAL_EDITOR_MODULES "Target names that should be linked into the editor")
option(GODOT_EXTERNAL_RUNTIME_MODULES "Target names that should be linked into runtimes (including the editor)")

set(GODOT_ENGINE_ROOT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_BINARY_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(GODOT_SOURCE_ROOT_THIRDPARTY_DIR ${GODOT_ENGINE_ROOT_DIRECTORY}/thirdparty)
set(GODOT_MODULES_BASE_DIR ${GODOT_ENGINE_ROOT_DIRECTORY}/modules)

set(OS_ARCH "amd64")

string(REGEX MATCH "(arm64|aarch64)" IS_ARM "${CMAKE_SYSTEM_PROCESSOR}")
if (IS_ARM)
    set(OS_ARCH "arm64")
endif ()

add_compile_definitions(TOOLS_ENABLED MACOS_ENABLED)

# IF debug_features
add_compile_definitions($<${WANT_DEBUG_FEATURES}:DEBUG_ENABLED>)
add_compile_options(-fobjc-arc)

message(STATUS "Build architecture: ${OS_ARCH}")

####################################
#Step 2: Add any custom modules
####################################

find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
set(GODOT_GENERATOR_SCRIPT ${CMAKE_SOURCE_DIR}/cmake_builders.py)

include(GodotGenerators.cmake)
include(Godot.cmake)

####################################
#Step 4: Generate the actual project
####################################
set(GODOT_ROOT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

#Bring in the base operating system platform
set(GODOT_SOURCE_ROOT_PLATFORM_DIR ${GODOT_ENGINE_ROOT_DIRECTORY}/platform)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(LinuxPlatform.cmake)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(WindowsPlatform.cmake)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(MacosPlatform.cmake)
endif ()

if (${WANT_VULKAN})
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(Vulkan REQUIRED)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        message(CHECK_START "Checking support for MoltenVK")
        if (EXISTS $ENV{MOLTENVK_SDK}/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a)
            set(MOLTENVK_ROOT $ENV{MOLTENVK_SDK}/MoltenVK.xcframework)
            add_library(MoltenVK STATIC IMPORTED)
            set_target_properties(
                    MoltenVK PROPERTIES
                    IMPORTED_LOCATION ${MOLTENVK_ROOT}/macos-arm64_x86_64/libMoltenVK.a
                    INTERFACE_INCLUDE_DIRECTORIES "${MOLTENVK_ROOT}/include/MoltenVK;${MOLTENVK_ROOT}/include/vulkan"
            )
            set(VULKAN_FOUND 1)
            message(CHECK_PASS "Found: ${MOLTENVK_ROOT}")
        else ()
            message(CHECK_FAIL "Unable to find MoltenVK")
            message(FATAL_ERROR "Set MOLTENVK_SDK to MoltenVK path")
        endif ()
    else ()
        # Windows relies on Volk alone.
        set(VULKAN_FOUND 1)
    endif ()
endif ()

if (${WANT_OPENGL})
    find_package(OpenGL REQUIRED)
endif ()

if (${WANT_DBUS})
    find_package(DBUS)
endif ()

if (${WANT_ALSA})
    find_package(ALSA)
endif ()

if (${WANT_PULSEAUDIO})
    find_package(PulseAudio)
endif ()

if (${WANT_X11})
    find_package(X11)
endif ()

if (${WANT_FONTCONFIG})
    find_package(Fontconfig)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(platform/linuxbsd)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(platform/windows)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(platform/macos)
endif ()

add_subdirectory(doc)
add_subdirectory(thirdparty)
add_subdirectory(drivers)
add_subdirectory(core)
add_subdirectory(servers)
add_subdirectory(scene)
add_subdirectory(modules)
add_subdirectory(editor)
add_subdirectory(main)
