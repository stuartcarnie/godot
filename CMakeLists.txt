cmake_minimum_required(VERSION 3.26 FATAL_ERROR)


list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)
set(GODOT_ENV_FILE ${CMAKE_BINARY_DIR}/cmake_env.json)

include(GodotUtils)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(godot
        HOMEPAGE_URL https://godotengine.org
        LANGUAGES C CXX OBJC OBJCXX
)

###
# Apple Specific
###

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "6N425KH923")
set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION[arch=*64] "NO")
set(CMAKE_XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION)

include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#########
# OPTIONS
#########

# Target build options

set(GODOT_TARGET "editor" CACHE STRING "Compilation target")
set(GODOT_TARGETS editor template_release template_debug)
set_property(CACHE GODOT_TARGET PROPERTY STRINGS ${GODOT_TARGETS})
option(ENABLE_THREADS "Enable threading support" ON)

# Components

set(PRECISION "single" CACHE STRING "Set the floating-point precision level")
set_property(CACHE PRECISION PROPERTY STRINGS single double)
option(ENABLE_MINIZIP "Enable ZIP archive support using minizip" ON)
option(ENABLE_BROTLI "Enable Brotli for decompresson and WOFF2 fonts support" ON)
option(DISABLE_EXCEPTIONS "Force disabling exception handling code" ON)

# Advanced options

option(ENABLE_DEV_BUILD "Developer build with dev-only debugging code (DEV_ENABLED)" OFF)
set(WARNINGS "all" CACHE STRING "Set the warning level")
set_property(CACHE WARNINGS PROPERTY STRINGS extra all moderate no)
option(ENABLE_WERROR "Treat compiler warnings as errors" OFF)

# Third party libraries

cmake_dependent_option(BUILTIN_BROTLI "Use the built-in Brotli library" ON "ENABLE_BROTLI" OFF)

#########
# Configure variables and build from options
#########

if (ENABLE_THREADS)
    set(WANT_THREADS 1)
else ()
    set(WANT_THREADS 0)
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
        add_compile_options(-Wall -Wextra -Wwrite-strings -Wno-unused-parameter ${COMMON_WARNINGS})
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy -Wnon-virtual-dtor>)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-Walloc-zero -Wduplicated-branches -Wduplicated-cond -Wstringop-overflow=4)
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wplacement-new=1>)
            # Need to fix a warning with AudioServer lambdas before enabling.
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "9")
                add_compile_options(-Wattribute-alias=2)
            endif ()
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11")
                # Broke on MethodBind templates before GCC 11.
                add_compile_options(-Wlogical-op)
            endif ()
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang$" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
            add_compile_options(-Wimplicit-fallthrough)
        endif ()
    elseif (WARNINGS STREQUAL "all")
        add_compile_options(-Wall ${COMMON_WARNINGS})
    elseif (WARNINGS STREQUAL "moderate")
        add_compile_options(-Wall -Wno-unused ${COMMON_WARNINGS})
    else () # no
        add_compile_options(-w)
    endif ()

    if (ENABLE_WERROR)
        add_compile_options(-Werror)
    endif ()
endif ()


if (ENABLE_BROTLI AND NOT BUILTIN_BROTLI)
    find_package(Brotli REQUIRED)
endif ()

if (ENABLE_DEV_BUILD)
    message(STATUS "Enabling developer build settings")
    add_compile_definitions(DEV_ENABLED)
    set(WANT_DEV_BUILD 1)
else ()
    set(WANT_DEV_BUILD 0)
endif ()

if (NOT GODOT_TARGET IN_LIST GODOT_TARGETS)
    message(FATAL_ERROR "Invalid GODOT_TARGET: ${GODOT_TARGET}. Expect editor, template_release or template_debug")
endif ()

if (GODOT_TARGET STREQUAL "editor")
    message(STATUS "Building editor")
    set(WANT_EDITOR 1)
else ()
    message(STATUS "Building runtime")
    set(WANT_EDITOR 0)
endif ()

if (GODOT_TARGET MATCHES "^(editor|template_debug)$")
    message(STATUS "Enabling debug features")
    set(WANT_DEBUG_FEATURES 1)
else ()
    set(WANT_DEBUG_FEATURES 0)
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
)

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
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(WindowsPlatform.cmake)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(MacosPlatform.cmake)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    include(IosPlatform.cmake)
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
        find_library(MOLTENVK_FOUND MoltenVK)
        set(VULKAN_FOUND 1)
    else ()
        # Windows relies on Volk alone.
        set(VULKAN_FOUND 1)
    endif ()
endif ()

if (${WANT_OPENGL})
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        find_package(OpenGL REQUIRED)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        find_package(OpenGLES3)
        set(OPENGL_FOUND 1)
    endif ()
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
endif ()
add_subdirectory(main)

block(SCOPE_FOR VARIABLES)
    macro(json_bool VAR VAL)
        if (${VAL})
            set(${VAR} true)
        else ()
            set(${VAR} false)
        endif ()
    endmacro()
    json_bool(EDITOR_BUILD ${WANT_EDITOR})
    json_bool(WANT_OPENXR_B ${WANT_OPENXR})
    json_bool(WANT_VULKAN_B ${WANT_VULKAN})
    json_bool(WANT_METAL_B ${WANT_METAL})
    configure_file(cmake_env.json.in cmake_env.json ESCAPE_QUOTES)
endblock()
