set(SMAA_AREA_TEXTURE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/AreaTex.png)
set(SMAA_AREA_TEXTURE_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/servers/rendering/renderer_rd/effects/smaa_area_tex.gen.h)
generate_file(make_smaa_areatex ${SMAA_AREA_TEXTURE_SOURCE} ${SMAA_AREA_TEXTURE_OUTPUT})

set(SMAA_SEARCH_TEXTURE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/SearchTex.png)
set(SMAA_SEARCH_TEXTURE_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/servers/rendering/renderer_rd/effects/smaa_search_tex.gen.h)
generate_file(make_smaa_searchtex ${SMAA_SEARCH_TEXTURE_SOURCE} ${SMAA_SEARCH_TEXTURE_OUTPUT})

add_library(smaa INTERFACE)
target_sources(smaa
        PUBLIC
        ${SMAA_AREA_TEXTURE_OUTPUT}
        ${SMAA_SEARCH_TEXTURE_OUTPUT}
)
