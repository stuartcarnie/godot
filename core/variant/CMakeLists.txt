godot_add_library(godot-variant FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-variant PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-variant PRIVATE
        array.cpp
        array.h
        binder_common.h
        callable.cpp
        callable.h
        callable_bind.cpp
        callable_bind.h
        container_type_validate.h
        dictionary.cpp
        dictionary.h
        method_ptrcall.h
        native_ptr.h
        type_info.h
        typed_array.h
        variant.cpp
        variant.h
        variant_call.cpp
        variant_callable.cpp
        variant_callable.h
        variant_construct.cpp
        variant_construct.h
        variant_destruct.cpp
        variant_destruct.h
        variant_internal.h
        variant_op.cpp
        variant_op.h
        variant_parser.cpp
        variant_parser.h
        variant_setget.cpp
        variant_setget.h
        variant_utility.cpp
        variant_utility.h
)
# [[[end]]]
target_link_libraries(godot-variant PUBLIC godot-object)