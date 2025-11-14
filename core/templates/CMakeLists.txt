# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        a_hash_map.cpp
        a_hash_map.h
        bin_sorted_array.h
        bit_field.h
        command_queue_mt.h
        cowdata.h
        fixed_vector.h
        hash_map.h
        hash_set.h
        hashfuncs.cpp
        hashfuncs.h
        interpolated_property.cpp
        interpolated_property.h
        iterable.h
        list.h
        local_vector.h
        lru.h
        paged_allocator.h
        paged_array.h
        pair.h
        pass_func.h
        pooled_list.h
        rb_map.h
        rb_set.h
        rid.h
        rid_owner.h
        ring_buffer.h
        safe_list.h
        safe_refcount.h
        self_list.h
        simple_type.h
        sort_array.h
        sort_list.h
        span.h
        tuple.h
        vector.h
        vset.h
)
# [[[end]]]
