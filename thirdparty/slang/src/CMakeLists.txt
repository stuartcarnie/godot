cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(slang LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

include(ICU.cmake)
include(GLSLANG.cmake)
include(SPIRV.cmake)

add_library(slang STATIC)
target_sources(slang PRIVATE
        ConfigFile.cpp
        ConfigFile.h
        ShaderModel.cpp
        ShaderModel.h
        shader_reflection.cpp
        shader_reflection.h
        shader_pass_reflection.cpp
        shader_pass_reflection.h
        shader_symbols.cpp
        shader_symbols.h
        ShaderParameter.h
        ShaderPass.cpp
        ShaderPass.h
        ShaderPassCompiler.cpp
        ShaderPassCompiler.h
        SlangShader.cpp
        SlangShader.h
        SourceParser.cpp
        SourceParser.h
        compiled.cpp
        compiled.h
        error.cpp
        error.h
        scanner.cpp
        scanner.h
        thirdparty/simdutf/simdutf.cpp
        thirdparty/simdutf/simdutf.h
        u32string.cpp
        u32string.h
)

target_link_libraries(slang PRIVATE icu4c glslang spirv-cross)
target_compile_features(slang PUBLIC cxx_std_20)

add_executable(slang_test test.cpp)
target_link_libraries(slang_test PRIVATE slang icu4c glslang spirv-cross)
