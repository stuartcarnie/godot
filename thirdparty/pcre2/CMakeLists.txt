godot_add_library(pcre2 FOLDER thirdparty STATIC)
target_sources(pcre2 PRIVATE
        src/pcre2_convert.c
        src/pcre2_auto_possess.c
        src/pcre2_jit_neon_inc.h
        src/pcre2_xclass.c
        src/pcre2_newline.c
        src/pcre2_extuni.c
        src/pcre2_ucd.c
        src/pcre2_substring.c
        src/pcre2_config.c
        src/pcre2_maketables.c
        src/pcre2_ucptables.c
        src/pcre2_serialize.c
        src/pcre2_ord2utf.c
        src/pcre2_pattern_info.c
        src/config.h
        src/pcre2_compile.c
        src/pcre2_context.c
        src/pcre2_match.c
        src/pcre2.h
        src/pcre2_jit_compile.c
        src/pcre2_study.c
        src/pcre2_internal.h
        src/pcre2_tables.c
        src/pcre2_valid_utf.c
        src/pcre2_chartables.c
        src/pcre2_match_data.c
        src/pcre2_ucp.h
        src/pcre2_dfa_match.c
        src/pcre2_intmodedep.h
        src/pcre2_substitute.c
        src/pcre2_script_run.c
        src/pcre2_error.c
        src/pcre2_find_bracket.c
        src/pcre2_string_utils.c
)
target_precompile_headers(pcre2 PRIVATE src/config.h src/sljit/sljitConfigInternal.h)
target_compile_definitions(pcre2 PUBLIC
                                #PCRE2_CODE_UNIT_WIDTH=16
                                #this may also be needed
                                PCRE2_CODE_UNIT_WIDTH=32
                                HAVE_CONFIG_H=1
                                SLJIT_VERBOSE=0
                                PCRE2_STATIC)
target_include_directories(pcre2 PUBLIC src)
