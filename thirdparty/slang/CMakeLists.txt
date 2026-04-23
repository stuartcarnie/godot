godot_add_library(slang FOLDER thirdparty STATIC)
target_sources(slang PRIVATE
        src/ConfigFile.cpp
        src/ConfigFile.h
        src/ShaderModel.cpp
        src/ShaderModel.h
        src/shader_reflection.cpp
        src/shader_reflection.h
        src/shader_pass_reflection.cpp
        src/shader_pass_reflection.h
        src/shader_symbols.cpp
        src/shader_symbols.h
        src/ShaderParameter.h
        src/ShaderPass.cpp
        src/ShaderPass.h
        src/ShaderPassCompiler.cpp
        src/ShaderPassCompiler.h
        src/SlangShader.cpp
        src/SlangShader.h
        src/SourceParser.cpp
        src/SourceParser.h
        src/compiled.cpp
        src/compiled.h
        src/error.cpp
        src/error.h
        src/scanner.cpp
        src/scanner.h
        src/thirdparty/simdutf/simdutf.cpp
        src/thirdparty/simdutf/simdutf.h
        src/u32string.cpp
        src/u32string.h
)
target_include_directories(slang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(slang PUBLIC godot-glslang spirv-cross icu4c)
target_compile_features(slang PUBLIC cxx_std_20)
target_compile_options(slang PRIVATE -fexceptions)
set_target_properties(slang PROPERTIES CXX_STANDARD 20 C_STANDARD 11)
