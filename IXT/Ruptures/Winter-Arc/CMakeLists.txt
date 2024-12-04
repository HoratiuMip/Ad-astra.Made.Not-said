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
)
target_link_libraries( ${WARC_LIB} PUBLIC "IXT" )
target_include_directories( ${WARC_LIB} PUBLIC ${WINTER_ARC_SOFT_DIR} )

set( _warc_lib_compile_definitions
    "WARC_RUPTURE_ROOT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/\""
    "WARC_RUPTURE_IMM_ROOT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/imm/\""
    "WARC_RUPTURE_DATA_ROOT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\""
)

if( WARC_INET_TLS )
    list( APPEND _warc_lib_compile_definitions "WARC_INET_TLS=1" )
else()
    list( APPEND _warc_lib_compile_definitions "WARC_INET_TLS=0" )
endif()

target_compile_definitions( ${WARC_LIB} PUBLIC ${_warc_lib_compile_definitions} )


add_executable( ${PROJECT_NAME} "soft/main.cpp" )
target_link_libraries( ${PROJECT_NAME} PUBLIC ${WARC_LIB} )

if( WARC_INET_TLS )
    target_link_libraries( ${PROJECT_NAME} PUBLIC $CACHE{IXT_OPEN_SSL_LIBS} )
endif()