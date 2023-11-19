godot_add_library(godot-templates FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-templates PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-templates PRIVATE
        bin_sorted_array.h
        command_queue_mt.cpp
        command_queue_mt.h
        cowdata.h
        hash_map.h
        hash_set.h
        hashfuncs.h
        list.h
        local_vector.h
        lru.h
        oa_hash_map.h
        paged_allocator.h
        paged_array.h
        pair.h
        pass_func.h
        pooled_list.h
        rb_map.h
        rb_set.h
        rid.h
        rid_owner.cpp
        rid_owner.h
        ring_buffer.h
        safe_list.h
        safe_refcount.h
        search_array.h
        self_list.h
        simple_type.h
        sort_array.h
        vector.h
        vmap.h
        vset.h
)
# [[[end]]]
target_link_libraries(godot-templates PUBLIC godot-object)
