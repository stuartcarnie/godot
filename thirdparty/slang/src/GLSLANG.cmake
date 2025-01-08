add_library(glslang STATIC)
target_sources(glslang PRIVATE
        ../../glslang/glslang/GenericCodeGen/CodeGen.cpp
        ../../glslang/glslang/GenericCodeGen/Link.cpp
        ../../glslang/glslang/MachineIndependent/Constant.cpp
        ../../glslang/glslang/MachineIndependent/InfoSink.cpp
        ../../glslang/glslang/MachineIndependent/Initialize.cpp
        ../../glslang/glslang/MachineIndependent/IntermTraverse.cpp
        ../../glslang/glslang/MachineIndependent/Intermediate.cpp
        ../../glslang/glslang/MachineIndependent/ParseContextBase.cpp
        ../../glslang/glslang/MachineIndependent/ParseHelper.cpp
        ../../glslang/glslang/MachineIndependent/PoolAlloc.cpp
        ../../glslang/glslang/MachineIndependent/RemoveTree.cpp
        ../../glslang/glslang/MachineIndependent/Scan.cpp
        ../../glslang/glslang/MachineIndependent/ShaderLang.cpp
        ../../glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp
        ../../glslang/glslang/MachineIndependent/SymbolTable.cpp
        ../../glslang/glslang/MachineIndependent/Versions.cpp
        ../../glslang/glslang/MachineIndependent/attribute.cpp
        ../../glslang/glslang/MachineIndependent/glslang_tab.cpp
        ../../glslang/glslang/MachineIndependent/intermOut.cpp
        ../../glslang/glslang/MachineIndependent/iomapper.cpp
        ../../glslang/glslang/MachineIndependent/limits.cpp
        ../../glslang/glslang/MachineIndependent/linkValidate.cpp
        ../../glslang/glslang/MachineIndependent/parseConst.cpp
        ../../glslang/glslang/MachineIndependent/preprocessor/Pp.cpp
        ../../glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp
        ../../glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp
        ../../glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp
        ../../glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp
        ../../glslang/glslang/MachineIndependent/propagateNoContraction.cpp
        ../../glslang/glslang/MachineIndependent/reflection.cpp
        ../../glslang/glslang/ResourceLimits/ResourceLimits.cpp
        ../../glslang/SPIRV/disassemble.cpp
        ../../glslang/SPIRV/doc.cpp
        ../../glslang/SPIRV/GlslangToSpv.cpp
        ../../glslang/SPIRV/InReadableOrder.cpp
        ../../glslang/SPIRV/Logger.cpp
        ../../glslang/SPIRV/SpvBuilder.cpp
        ../../glslang/SPIRV/SpvPostProcess.cpp
        ../../glslang/SPIRV/SPVRemapper.cpp
        ../../glslang/SPIRV/SpvTools.cpp

        ../../glslang/$<IF:$<PLATFORM_ID:Windows>,glslang/OSDependent/Windows/ossource.cpp,glslang/OSDependent/Unix/ossource.cpp>
)

target_include_directories(glslang PUBLIC ../../glslang ../..)

target_compile_options(glslang PRIVATE $<$<CONFIG:Debug>:-O2>)
