set(PCRE2_SOURCES
        src/pcre2_auto_possess.c
        src/pcre2_chartables.c
        src/pcre2_chkdint.c
        src/pcre2_compile.c
        src/pcre2_config.c
        src/pcre2_context.c
        src/pcre2_convert.c
        src/pcre2_dfa_match.c
        src/pcre2_error.c
        src/pcre2_extuni.c
        src/pcre2_find_bracket.c
        src/pcre2_jit_compile.c
        # "pcre2_jit_match.c", "pcre2_jit_misc.c", # these files are included in pcre2_jit_compile.c.
        src/pcre2_maketables.c
        src/pcre2_match.c
        src/pcre2_match_data.c
        src/pcre2_newline.c
        src/pcre2_ord2utf.c
        src/pcre2_pattern_info.c
        src/pcre2_script_run.c
        src/pcre2_serialize.c
        src/pcre2_string_utils.c
        src/pcre2_study.c
        src/pcre2_substitute.c
        src/pcre2_substring.c
        src/pcre2_tables.c
        src/pcre2_ucd.c
        src/pcre2_valid_utf.c
        src/pcre2_xclass.c
)
macro(add_pcre WIDTH)
    godot_add_library(pcre2_w${WIDTH} FOLDER thirdparty STATIC)
    target_sources(pcre2_w${WIDTH} PRIVATE ${PCRE2_SOURCES})
    # target_precompile_headers(pcre2 PRIVATE src/config.h src/sljit/sljitConfigInternal.h)
    target_compile_definitions(pcre2_w${WIDTH}
            PRIVATE
            SUPPORT_UNICODE
            PCRE2_CODE_UNIT_WIDTH=${WIDTH}
            HAVE_CONFIG_H
            PUBLIC
            PCRE2_STATIC)
    target_include_directories(pcre2_w${WIDTH} PUBLIC src)
endmacro()

add_pcre(16)
add_pcre(32)

