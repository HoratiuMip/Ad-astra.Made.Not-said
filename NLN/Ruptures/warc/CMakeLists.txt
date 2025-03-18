cmake_minimum_required( VERSION 3.30.0 )


option( WARC_INET_TLS "WARC_INET_TLS" OFF )


project(
    "warc"
    VERSION 1.0.0
    LANGUAGES C CXX
)

set( WARC_LIB "warclib" )
set( WINTER_ARC_SOFT_DIR "${CMAKE_CURRENT_LIST_DIR}/soft" )

add_library( ${WARC_LIB} STATIC
    "${WINTER_ARC_SOFT_DIR}/inet-tls.cpp"
    "${WINTER_ARC_SOFT_DIR}/n2yo.cpp"
    "${WINTER_ARC_SOFT_DIR}/earth-imm.cpp"
    "${WINTER_ARC_SOFT_DIR}/warc-main.cpp"
    "${WINTER_ARC_SOFT_DIR}/astro.cpp"
    "${WINTER_ARC_SOFT_DIR}/dev.cpp"
)
target_link_libraries( ${WARC_LIB} PUBLIC "NLN" )
target_include_directories( ${WARC_LIB} PUBLIC ${WINTER_ARC_SOFT_DIR} )


if( WARC_INET_TLS )
    list( APPEND _warc_lib_compile_definitions "WARC_INET_TLS=1" )
else()
    list( APPEND _warc_lib_compile_definitions "WARC_INET_TLS=0" )
endif()

target_compile_definitions( ${WARC_LIB} PUBLIC ${_warc_lib_compile_definitions} )


add_executable( ${PROJECT_NAME} "${WINTER_ARC_SOFT_DIR}/main.cpp" )
target_link_libraries( ${PROJECT_NAME} PUBLIC ${WARC_LIB} )
target_link_libraries( ${PROJECT_NAME} PUBLIC "NLN" )

if( WARC_INET_TLS )
    target_link_libraries( ${PROJECT_NAME} PUBLIC $CACHE{NLN_OPEN_SSL_LIBS} )
endif()