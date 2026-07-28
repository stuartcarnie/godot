cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)
set(GODOT_ENV_FILE ${CMAKE_BINARY_DIR}/cmake_env.json)

include(GodotUtils)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(godot
        HOMEPAGE_URL https://godotengine.org
        LANGUAGES C CXX OBJC OBJCXX Swift
)

set(CMAKE_Swift_LANGUAGE_VERSION 6)

###
# Apple Specific
###
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0")
elseif (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "26.0")
elseif (CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "26.0")
else ()
    # skip setting deployment target for other platforms
endif ()

# Configure the Xcode project
set(CMAKE_XCODE_ATTRIBUTE_OSX_DEPLOYMENT_TARGET 13.0)
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 15.0)
set(CMAKE_XCODE_ATTRIBUTE_VISIONOS_DEPLOYMENT_TARGET 26.0)
set(CMAKE_XCODE_ATTRIBUTE_TVOS_DEPLOYMENT_TARGET 26.0)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "6N425KH923")
set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION[arch=*64] "NO")
set(CMAKE_XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION)
set(CMAKE_XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK "YES")
set(CMAKE_XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=Debug] "0")
set(CMAKE_XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=RelWithDebInfo] "2")
set(CMAKE_XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=Release] "3")
set(CMAKE_XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=MinSizeRel] "s")
set(CMAKE_XCODE_XCCONFIG godot.xcconfig)

include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

#########
# OPTIONS
#########

# Target build options

set(GODOT_TARGET "editor" CACHE STRING "Compilation target")
set(GODOT_TARGETS editor template_release template_debug)
set_property(CACHE GODOT_TARGET PROPERTY STRINGS ${GODOT_TARGETS})
set(LIBRARY_TYPE "executable" CACHE STRING "Build library type")
set(LIBRARY_TYPES executable static_library shared_library)
set_property(CACHE LIBRARY_TYPE PROPERTY STRINGS ${LIBRARY_TYPES})
option(ENABLE_THREADS "Enable threading support" ON)
option(ENABLE_ENGINE_UPDATE_CHECK "Enable engine update checks in the Project Manager" ON)

# Components

set(PRECISION "single" CACHE STRING "Set the floating-point precision level")
set_property(CACHE PRECISION PROPERTY STRINGS single double)
option(ENABLE_MINIZIP "Enable ZIP archive support using minizip" ON)
option(ENABLE_BROTLI "Enable Brotli for decompresson and WOFF2 fonts support" ON)
option(ENABLE_SDL "Enable the SDL3 input driver" ON)
option(DISABLE_EXCEPTIONS "Force disabling exception handling code" ON)

option(DISABLE_3D "Disable 3D nodes for a smaller executable" OFF)
option(DISABLE_PHYSICS_2D "Disable 2D physics nodes and server" OFF)
option(DISABLE_PHYSICS_3D "Disable 3D physics nodes and server" OFF)
option(DISABLE_NAVIGATION_2D "Disable 2D navigation features" OFF)
option(DISABLE_NAVIGATION_3D "Disable 3D navigation features" OFF)
option(DISABLE_XR "Disable XR nodes and server" OFF)
# TODO(sgc): Add support to enable compiling with AccessKit
option(ENABLE_ACCESSKIT "Use AccessKit C SDK" OFF)

# Profiling
set(PROFILER "none" CACHE STRING "Specify a profiler backend to use")
set_property(CACHE PROFILER PROPERTY STRINGS none tracy perfetto instruments)

set(PROFILER_PATH "" CACHE STRING "Path to the Profiler framework.")
option(PROFILER_SAMPLE_CALLSTACK "Profile random samples application-wide using a callstack based sampler." OFF)
option(PROFILER_TRACK_MEMORY "Profile memory allocations, if the profiler supports it." OFF)
option(PROFILER_RECORD_ON_DEMAND "Record only when the profiler is connected, if the profiler supports it. In Tracy, this configures TRACY_ON_DEMAND, which has a performance impact if enabled." OFF)

# Determine profiler backend

set(GODOT_CORE_PROFILER_BACKEND "")
set(GODOT_CORE_PROFILER_SOURCE_FILES "")
set(GODOT_CORE_PROFILER_INCLUDE_DIRS "")
set(GODOT_CORE_PROFILER_DEFINITIONS "")

if (PROFILER STREQUAL "instruments")
    set(GODOT_CORE_PROFILER_BACKEND instruments)
elseif (PROFILER STREQUAL "tracy")
    set(_godot_profiler_path "${PROFILER_PATH}")
    if (NOT IS_ABSOLUTE "${_godot_profiler_path}")
        get_filename_component(_godot_profiler_path "${_godot_profiler_path}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    endif ()
    if (NOT IS_DIRECTORY "${_godot_profiler_path}")
        message(FATAL_ERROR "PROFILER_PATH must be empty or point to a directory.")
    endif ()

    if (EXISTS "${_godot_profiler_path}/public/TracyClient.cpp")
        set(_godot_profiler_include_dir "${_godot_profiler_path}/public")
        set(_godot_profiler_source "${_godot_profiler_path}/public/TracyClient.cpp")
    elseif (EXISTS "${_godot_profiler_path}/TracyClient.cpp")
        set(_godot_profiler_include_dir "${_godot_profiler_path}")
        set(_godot_profiler_source "${_godot_profiler_path}/TracyClient.cpp")
    else ()
        message(FATAL_ERROR "Unrecognized PROFILER_PATH option. Please set a path to either tracy or perfetto.")
    endif ()

    set(GODOT_CORE_PROFILER_BACKEND tracy)
    list(APPEND GODOT_CORE_PROFILER_SOURCE_FILES "${_godot_profiler_source}")
    list(APPEND GODOT_CORE_PROFILER_INCLUDE_DIRS "${_godot_profiler_include_dir}")
elseif (PROFILER STREQUAL "perfetto")
    set(_godot_profiler_path "${PROFILER_PATH}")
    if (NOT IS_ABSOLUTE "${_godot_profiler_path}")
        get_filename_component(_godot_profiler_path "${_godot_profiler_path}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
    endif ()
    if (NOT IS_DIRECTORY "${_godot_profiler_path}")
        message(FATAL_ERROR "PROFILER_PATH must be empty or point to a directory.")
    endif ()

    if (EXISTS "${_godot_profiler_path}/sdk/perfetto.cc")
        set(_godot_profiler_include_dir "${_godot_profiler_path}/sdk")
        set(_godot_profiler_source "${_godot_profiler_path}/sdk/perfetto.cc")
    elseif (EXISTS "${_godot_profiler_path}/perfetto.cc")
        set(_godot_profiler_include_dir "${_godot_profiler_path}")
        set(_godot_profiler_source "${_godot_profiler_path}/perfetto.cc")
    else ()
        message(FATAL_ERROR "Unrecognized PROFILER_PATH option. Please set a path to either tracy or perfetto.")
    endif ()

    set(GODOT_CORE_PROFILER_BACKEND perfetto)
    list(APPEND GODOT_CORE_PROFILER_SOURCE_FILES "${_godot_profiler_source}")
    list(APPEND GODOT_CORE_PROFILER_INCLUDE_DIRS "${_godot_profiler_include_dir}")
endif ()

if (GODOT_CORE_PROFILER_BACKEND STREQUAL "instruments")
    set(GODOT_USE_INSTRUMENTS 1)
    if (PROFILER_SAMPLE_CALLSTACK)
        set(INSTRUMENTS_SAMPLE_CALLSTACKS 1)
    endif ()
    set(WANT_PROFILER 1)
elseif (GODOT_CORE_PROFILER_BACKEND STREQUAL "tracy")
    set(GODOT_USE_TRACY 1)
    if (PROFILER_SAMPLE_CALLSTACK)
        set(TRACY_CALLSTACK 62)
    endif ()
    if (PROFILER_TRACK_MEMORY)
        set(GODOT_PROFILER_TRACK_MEMORY 1)
    endif ()
    if (PROFILER_RECORD_ON_DEMAND)
        set(TRACY_ON_DEMAND 1)
    endif ()
    set(WANT_PROFILER 1)
elseif (GODOT_CORE_PROFILER_BACKEND STREQUAL "perfetto")
    set(GODOT_USE_PERFETTO 1)
    if (PROFILER_SAMPLE_CALLSTACK)
        message(FATAL_ERROR "Perfetto does not support call stack sampling.")
    endif ()
    set(WANT_PROFILER 1)
elseif (PROFILER_SAMPLE_CALLSTACK)
    message(WARNING "PROFILER_SAMPLE_CALLSTACK is ignored because no profiler backend is configured.")
    set(WANT_PROFILER 0)
endif ()

unset(_godot_profiler_include_dir)
unset(_godot_profiler_source)
unset(_godot_profiler_path)


if (DISABLE_XR)
    add_compile_definitions(XR_DISABLED)
endif ()

# Custom modules

option(ENABLE_RETROFX "Enable RetroFX module" OFF)

if (ENABLE_RETROFX)
    set(WANT_RETROFX 1)
else ()
    set(WANT_RETROFX 0)
endif ()

if (ENABLE_SDL AND CMAKE_SYSTEM_NAME MATCHES "^(Linux|Windows|Darwin|iOS|tvOS|visionOS)$")
    set(WANT_SDL 1)
else ()
    set(WANT_SDL 0)
endif ()

# Advanced options

set(DEV_BUILD_LEVEL none CACHE STRING "Developer build with dev-only debugging code (DEV_ENABLED)")
set(DEV_BUILD_LIST none debug all)
set_property(CACHE DEV_BUILD_LEVEL PROPERTY STRINGS ${DEV_BUILD_LIST})
set(WARNINGS "all" CACHE STRING "Set the warning level")
set_property(CACHE WARNINGS PROPERTY STRINGS extra all moderate no)
option(ENABLE_WERROR "Treat compiler warnings as errors" OFF)
option(APPLE_BUNDLE_IDENTIFIER "Set the bundle identifier for macOS/iOS builds" "org.godotengine.godot")
mark_as_advanced(APPLE_BUNDLE_IDENTIFIER)
option(ENABLE_DYNAMIC_CORE_SHADERS "Load core shaders from disk instead of embedding them" OFF)
set(GODOT_PCK_FILE "" CACHE FILEPATH "Path to a Godot .pck file to bundle with the app (iOS/visionOS)")

if (ENABLE_DYNAMIC_CORE_SHADERS)
    set(WANT_DYNAMIC_CORE_SHADERS 1)
else ()
    set(WANT_DYNAMIC_CORE_SHADERS 0)
endif ()

# Third party libraries

cmake_dependent_option(BUILTIN_BROTLI "Use the built-in Brotli library" ON "ENABLE_BROTLI" OFF)

#########
# Configure variables and build from options
#########

if (ENABLE_TESTS)
    set(WANT_TESTS 1)
else ()
    set(WANT_TESTS 0)
endif ()

if (ENABLE_THREADS)
    set(WANT_THREADS 1)
else ()
    set(WANT_THREADS 0)
endif ()

if (DEV_BUILD_LEVEL STREQUAL "none")
    set(WANT_DEV_BUILD 0)
elseif (DEV_BUILD_LEVEL STREQUAL "debug")
    message(STATUS "Enabling developer build settings for Debug configuration")
    add_compile_definitions($<$<CONFIG:Debug>:DEV_ENABLED>)
    set(WANT_DEV_BUILD 1)
elseif (DEV_BUILD_LEVEL STREQUAL "all")
    message(STATUS "Enabling developer build settings for all configurations")
    add_compile_definitions(DEV_ENABLED)
    set(WANT_DEV_BUILD 1)
else ()
    message(FATAL_ERROR "Invalid DEV_BUILD_LEVEL: ${DEV_BUILD_LEVEL}. Expected ${DEV_BUILD_LIST}")
endif ()

if (WANT_DEV_BUILD AND NOT ${WARNINGS} STREQUAL "no")
    set(WARNINGS "extra")
    set(ENABLE_WERROR ON)
endif ()

# Configure compiler warnings
if (MSVC)
    if (WARNINGS STREQUAL "no")
        add_compile_options(/W)
    else ()
        if (WARNINGS STREQUAL "extra")
            add_compile_options(/W4)
        elseif (WARNINGS STREQUAL "all")
            # C4458 is like -Wshadow. Part of /W4 but let's apply it for the default /W3 too.
            add_compile_options(/W3 /w34458)
        elseif (WARNINGS STREQUAL "moderate")
            add_compile_options(/W2)
        endif ()
        # Disable warnings which we don't plan to fix.
        add_compile_options(
                /wd4100   # C4100 (unreferenced formal parameter): Doesn't play nice with polymorphism.
                /wd4127   # C4127 (conditional expression is constant)
                /wd4201   # C4201 (non-standard nameless struct/union): Only relevant for C89.
                /wd4244   # C4244 C4245 C4267 (narrowing conversions): Unavoidable at this scale.
                /wd4245
                /wd4267
                /wd4305   # C4305 (truncation): double to float or real_t, too hard to avoid.
                /wd4514   # C4514 (unreferenced inline function has been removed)
                /wd4714   # C4714 (function marked as __forceinline not inlined)
                /wd4820   # C4820 (padding added after construct)
        )

        if (ENABLE_WERROR)
            add_compile_options(/WX)
            add_link_options(/WX)
        endif ()
    endif ()
else ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(COMMON_WARNINGS -Wshadow -Wno-misleading-indentation)

        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7.0")
            # Bogus warning fixed in 8+.
            list(APPEND COMMON_WARNINGS -Wno-strict-overflow)
        endif ()
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
            # Regression in GCC 9/10, spams so much in our variadic templates
            # that we need to outright disable it.
            list(APPEND COMMON_WARNINGS -Wno-type-limits)
        endif ()
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0")
            # False positives in our error macros, see GH-58747.
            list(APPEND COMMON_WARNINGS -Wno-return-type)
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang$" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        list(APPEND COMMON_WARNINGS -Wshadow-field-in-constructor -Wshadow-uncaptured-local)
        # We often implement `operator<` for structs of pointers as a requirement
        # for putting them in `Set` or `Map`. We don't mind about unreliable ordering.
        list(APPEND COMMON_WARNINGS -Wno-ordered-compare-function-pointers)
    endif ()

    if (WARNINGS STREQUAL "extra")
        list(APPEND COMMON_WARNINGS -Wall -Wextra -Wwrite-strings -Wno-unused-parameter -Wctor-dtor-privacy -Wnon-virtual-dtor)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            list(APPEND COMMON_WARNINGS -Walloc-zero -Wduplicated-branches -Wduplicated-cond -Wstringop-overflow=4)
            list(APPEND COMMON_WARNINGS -Wplacement-new=1)
            # Need to fix a warning with AudioServer lambdas before enabling.
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "9")
                list(APPEND COMMON_WARNINGS -Wattribute-alias=2)
            endif ()
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11")
                # Broke on MethodBind templates before GCC 11.
                list(APPEND COMMON_WARNINGS -Wlogical-op)
            endif ()
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang$" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
            list(APPEND COMMON_WARNINGS -Wimplicit-fallthrough)
        endif ()
    elseif (WARNINGS STREQUAL "all")
        list(APPEND COMMON_WARNINGS -Wall)
    elseif (WARNINGS STREQUAL "moderate")
        list(APPEND COMMON_WARNINGS -Wall -Wno-unused)
    else () # no
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-w>)
    endif ()

    if (ENABLE_WERROR)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:-Werror>)
    endif ()

    add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:$<JOIN:${COMMON_WARNINGS},$<SEMICOLON>>>")
endif ()


if (ENABLE_BROTLI AND NOT BUILTIN_BROTLI)
    find_package(Brotli REQUIRED)
endif ()

if (NOT GODOT_TARGET IN_LIST GODOT_TARGETS)
    message(FATAL_ERROR "Invalid GODOT_TARGET: ${GODOT_TARGET}. Expect editor, template_release or template_debug")
endif ()

if (NOT LIBRARY_TYPE IN_LIST LIBRARY_TYPES)
    message(FATAL_ERROR "Invalid LIBRARY_TYPE: ${LIBRARY_TYPE}. Expect executable, static_library or shared_library")
endif ()

if (GODOT_TARGET STREQUAL "editor")
    message(STATUS "Building editor")
    set(WANT_EDITOR 1)
else ()
    message(STATUS "Building runtime")
    set(WANT_EDITOR 0)
endif ()

if (ENABLE_ENGINE_UPDATE_CHECK AND WANT_EDITOR)
    set(WANT_ENGINE_UPDATE_CHECK 1)
else ()
    set(WANT_ENGINE_UPDATE_CHECK 0)
endif ()

if (GODOT_TARGET MATCHES "^(editor|template_debug)$")
    message(STATUS "Enabling debug features")
    set(WANT_DEBUG_FEATURES 1)
else ()
    set(WANT_DEBUG_FEATURES 0)
endif ()

if (LIBRARY_TYPE STREQUAL "executable")
    set(WANT_LIBGODOT 0)
else ()
    message(STATUS "Building LibGodot ${LIBRARY_TYPE}")
    set(WANT_LIBGODOT 1)
endif ()

if (PRECISION STREQUAL "single")
    set(WANT_DOUBLE_PRECISION 0)
elseif (PRECISION STREQUAL "double")
    set(WANT_DOUBLE_PRECISION 1)
    message(STATUS "Use double precision")
else ()
    message(FATAL_ERROR "Invalid PRECISION: ${PRECISION}. Expect single or double")
endif ()

add_compile_definitions($<${WANT_DOUBLE_PRECISION}:REAL_T_IS_DOUBLE>)

add_compile_options(
        $<$<AND:$<BOOL:${DISABLE_EXCEPTIONS}>,$<COMPILE_LANGUAGE:CXX>>:-fno-exceptions>
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

add_compile_definitions(
        $<${WANT_EDITOR}:TOOLS_ENABLED>
        $<${WANT_THREADS}:THREADS_ENABLED>
        $<${WANT_DEBUG_FEATURES}:DEBUG_ENABLED>
        $<${WANT_LIBGODOT}:LIBGODOT_ENABLED>
)

add_compile_options($<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:-fobjc-arc>)

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
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(WindowsPlatform.cmake)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(MacosPlatform.cmake)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    include(IosPlatform.cmake)
elseif (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
    include(TvosPlatform.cmake)
elseif (CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    include(VisionosPlatform.cmake)
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
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
                    INTERFACE_INCLUDE_DIRECTORIES "$ENV{MOLTENVK_SDK}/include/MoltenVK;$ENV{MOLTENVK_SDK}/include/vulkan"
            )
            set(VULKAN_FOUND 1)
            message(CHECK_PASS "Found: ${MOLTENVK_ROOT}")
        else ()
            message(CHECK_FAIL "Unable to find MoltenVK")
            message(FATAL_ERROR "Set MOLTENVK_SDK to MoltenVK path")
        endif ()
    elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        find_library(MOLTENVK_FOUND MoltenVK REQUIRED)
        set(VULKAN_FOUND 1)
    else ()
        # Windows relies on Volk alone.
        set(VULKAN_FOUND 1)
    endif ()
else ()
    set(VULKAN_FOUND 0)
endif ()

if (${WANT_OPENGL})
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        find_package(OpenGL REQUIRED)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        find_package(OpenGLES3)
        set(OPENGL_FOUND 1)
    endif ()
else ()
    set(OPENGL_FOUND 0)
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
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(platform/windows)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(platform/macos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_subdirectory(platform/ios)
elseif (CMAKE_SYSTEM_NAME STREQUAL "tvOS")
    add_subdirectory(platform/tvos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    add_subdirectory(platform/visionos)
endif ()

if (WANT_EDITOR)
    add_subdirectory(doc)
endif ()
add_subdirectory(thirdparty)
add_subdirectory(drivers)
add_subdirectory(core)
add_subdirectory(servers)
add_subdirectory(scene)
add_subdirectory(modules)
if (WANT_EDITOR)
    add_subdirectory(editor)
    add_subdirectory(platform/ios/export)
    add_subdirectory(platform/visionos/export)
    add_subdirectory(platform/macos/export)
endif ()
add_subdirectory(platform/ios/api)
add_subdirectory(platform/visionos/api)
add_subdirectory(platform/android/api)
add_subdirectory(platform/web/api)
if (WANT_TESTS)
    add_subdirectory(tests)
endif ()
add_subdirectory(main)

function(json_set_bool OUT_VAR JSON_STR MEMBER BOOL_VAL)
    if (${BOOL_VAL})
        string(JSON _json SET "${JSON_STR}" ${MEMBER} true)
    else ()
        string(JSON _json SET "${JSON_STR}" ${MEMBER} false)
    endif ()
    set(${OUT_VAR} "${_json}" PARENT_SCOPE)
endfunction()

function(json_set_string OUT_VAR JSON_STR MEMBER STR_VAL)
    string(JSON _json SET "${JSON_STR}" ${MEMBER} "\"${STR_VAL}\"")
    set(${OUT_VAR} "${_json}" PARENT_SCOPE)
endfunction()

set(_json "{}")
json_set_string(_json "${_json}" platform "${GODOT_PLATFORM}")
json_set_string(_json "${_json}" arch "${OS_ARCH}")
json_set_string(_json "${_json}" library_type "${LIBRARY_TYPE}")
json_set_bool(_json "${_json}" editor_build ${WANT_EDITOR})
json_set_bool(_json "${_json}" modules_enabled_by_default TRUE)
json_set_bool(_json "${_json}" disable_3d FALSE)
json_set_bool(_json "${_json}" disable_physics_2d ${DISABLE_PHYSICS_2D})
json_set_bool(_json "${_json}" disable_physics_3d ${DISABLE_PHYSICS_3D})
json_set_bool(_json "${_json}" disable_navigation_2d ${DISABLE_NAVIGATION_2D})
json_set_bool(_json "${_json}" disable_navigation_3d ${DISABLE_NAVIGATION_3D})
json_set_bool(_json "${_json}" disable_xr ${DISABLE_XR})
json_set_bool(_json "${_json}" vulkan ${WANT_VULKAN})
json_set_bool(_json "${_json}" d3d12 FALSE)
json_set_bool(_json "${_json}" metal ${WANT_METAL})
json_set_bool(_json "${_json}" openxr ${WANT_OPENXR})
json_set_bool(_json "${_json}" module_retrofx_enabled ${WANT_RETROFX})
json_set_bool(_json "${_json}" opengl3 TRUE)
json_set_bool(_json "${_json}" minizip TRUE)
json_set_bool(_json "${_json}" builtin_certs TRUE)
json_set_string(_json "${_json}" system_certs_path "")
file(WRITE "${CMAKE_BINARY_DIR}/cmake_env.json" "${_json}\n")

# IMPORTANT: Must be last, so all targets have been added
#if (CMAKE_GENERATOR STREQUAL "Xcode")
#    set(GODOT_XCODE_INHERITED_ATTRIBUTES
#            GCC_OPTIMIZATION_LEVEL
#            GCC_GENERATE_DEBUGGING_SYMBOLS
#    )
#    function(_xcode_inherit_attributes dir)
#        get_property(tgts DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
#        foreach (t IN LISTS tgts)
#            get_target_property(type ${t} TYPE)
#            if (type STREQUAL "INTERFACE_LIBRARY" OR type STREQUAL "UTILITY")
#                continue()
#            endif ()
#            foreach (attr IN LISTS GODOT_XCODE_INHERITED_ATTRIBUTES)
#                get_target_property(keep ${t} GODOT_XCODE_KEEP_${attr})
#                if (NOT keep)
#                    set_target_properties(${t} PROPERTIES
#                            XCODE_ATTRIBUTE_${attr} "$(inherited)")
#                endif ()
#            endforeach ()
#        endforeach ()
#        get_property(subs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
#        foreach (s IN LISTS subs)
#            _xcode_inherit_attributes(${s})
#        endforeach ()
#    endfunction()
#    _xcode_inherit_attributes(${CMAKE_SOURCE_DIR})
#endif ()
