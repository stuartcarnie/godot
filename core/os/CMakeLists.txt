godot_add_library(godot-os FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-os PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-os PRIVATE
        condition_variable.h
        keyboard.cpp
        keyboard.h
        main_loop.cpp
        main_loop.h
        memory.cpp
        memory.h
        midi_driver.cpp
        midi_driver.h
        mutex.cpp
        mutex.h
        os.cpp
        os.h
        pool_allocator.cpp
        pool_allocator.h
        rw_lock.h
        safe_binary_mutex.h
        semaphore.h
        spin_lock.h
        thread.cpp
        thread.h
        thread_safe.cpp
        thread_safe.h
        time.cpp
        time.h
        time_enums.h
)
# [[[end]]]
target_link_libraries(godot-os PUBLIC godot-object)