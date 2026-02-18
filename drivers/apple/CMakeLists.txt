# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(drivers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(drivers PRIVATE
        apple_tracing.cpp
        apple_tracing.h
        foundation_helpers.h
        foundation_helpers.mm
        os_log_logger.cpp
        os_log_logger.h
        thread_apple.cpp
        thread_apple.h
)
# [[[end]]]

