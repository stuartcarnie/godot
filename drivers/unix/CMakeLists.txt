godot_add_library(driver-unix FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-unix PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-unix PRIVATE
        dir_access_unix.cpp
        dir_access_unix.h
        file_access_unix.cpp
        file_access_unix.h
        ip_unix.cpp
        ip_unix.h
        net_socket_posix.cpp
        net_socket_posix.h
        os_unix.cpp
        os_unix.h
        syslog_logger.cpp
        syslog_logger.h
        thread_posix.cpp
        thread_posix.h
)
# [[[end]]]
target_link_libraries(driver-unix PUBLIC godot-io)
