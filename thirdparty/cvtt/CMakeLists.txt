godot_add_library(cvtt FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(cvtt PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(cvtt PRIVATE
        ConvectionKernels.h
        ConvectionKernels_API.cpp
        ConvectionKernels_AggregatedError.h
        ConvectionKernels_BC67.cpp
        ConvectionKernels_BC67.h
        ConvectionKernels_BC6H_IO.cpp
        ConvectionKernels_BC6H_IO.h
        ConvectionKernels_BC7_Prio.h
        ConvectionKernels_BC7_PrioData.cpp
        ConvectionKernels_BC7_SingleColor.h
        ConvectionKernels_BCCommon.cpp
        ConvectionKernels_BCCommon.h
        ConvectionKernels_Config.h
        ConvectionKernels_ETC.cpp
        ConvectionKernels_ETC.h
        ConvectionKernels_ETC1.h
        ConvectionKernels_ETC2.h
        ConvectionKernels_ETC2_Rounding.h
        ConvectionKernels_EndpointRefiner.h
        ConvectionKernels_EndpointSelector.h
        ConvectionKernels_FakeBT709_Rounding.h
        ConvectionKernels_IndexSelector.cpp
        ConvectionKernels_IndexSelector.h
        ConvectionKernels_IndexSelectorHDR.h
        ConvectionKernels_PackedCovarianceMatrix.h
        ConvectionKernels_ParallelMath.h
        ConvectionKernels_S3TC.cpp
        ConvectionKernels_S3TC.h
        ConvectionKernels_S3TC_SingleColor.h
        ConvectionKernels_SingleFile.cpp
        ConvectionKernels_UnfinishedEndpoints.h
        ConvectionKernels_Util.cpp
        ConvectionKernels_Util.h
)
# [[[end]]]

target_include_directories(cvtt PUBLIC .)
