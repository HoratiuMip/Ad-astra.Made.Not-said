cmake_minimum_required( VERSION 3.30.0 )


option( WARC_MAIN "WARC_MAIN" OFF )
option( WARC_DB "WARC_DB" OFF )


set( WARC_SOFT_DIR "${CMAKE_CURRENT_LIST_DIR}/soft" )


if( WARC_MAIN )
    option( WARC_INET_TLS "WARC_INET_TLS" OFF )


    project(
        "warc"
        VERSION 1.0.0
        LANGUAGES C CXX
    )

    set( WARC_LIB "warclib" )

    add_library( ${WARC_LIB} STATIC
        "${WARC_SOFT_DIR}/inet-tls.cpp"
        "${WARC_SOFT_DIR}/n2yo.cpp"
        "${WARC_SOFT_DIR}/earth-imm.cpp"
        "${WARC_SOFT_DIR}/warc-main.cpp"
        "${WARC_SOFT_DIR}/astro.cpp"
        "${WARC_SOFT_DIR}/dev.cpp"
    )
    target_link_libraries( ${WARC_LIB} PUBLIC "IXN" )
    target_include_directories( ${WARC_LIB} PUBLIC ${WARC_SOFT_DIR} )


    if( WARC_INET_TLS )
        list( APPEND _warc_lib_compile_definitions "WARC_INET_TLS=1" )
    else()
        list( APPEND _warc_lib_compile_definitions "WARC_INET_TLS=0" )
    endif()

    target_compile_definitions( ${WARC_LIB} PUBLIC ${_warc_lib_compile_definitions} )


    add_executable( ${PROJECT_NAME} "${WARC_SOFT_DIR}/main.cpp" )
    target_link_libraries( ${PROJECT_NAME} PUBLIC ${WARC_LIB} )
    target_link_libraries( ${PROJECT_NAME} PUBLIC "IXN" )

    if( WARC_INET_TLS )
        target_link_libraries( ${PROJECT_NAME} PUBLIC $CACHE{IXN_OPEN_SSL_LIBS} )
    endif()

endif()

if( WARC_DB )
    project(
        "warc_db"
        VERSION 1.0.0
        LANGUAGES C CXX
    )   
    
    set( bsoncxx_DIR "C:/Hackerman/mongo-cxx-driver/lib/cmake/bsoncxx-4.0.0/" )
    set( mongocxx_DIR "C:/Hackerman/mongo-cxx-driver/lib/cmake/mongocxx-4.0.0/" )
    set( mongoc-1.0_DIR "C:/Hackerman/mongo-cxx-driver/lib/cmake/mongoc-1.0/" )
    set( bson-1.0_DIR "C:/Hackerman/mongo-cxx-driver/lib/cmake/bson-1.0/" )

    find_package( mongocxx REQUIRED )
    find_package( bsoncxx REQUIRED )
    find_package( mongoc-1.0 REQUIRED )
    find_package( bson-1.0 REQUIRED )

    add_executable( ${PROJECT_NAME} "${WARC_SOFT_DIR}/db_main.cpp" )

    target_include_directories( ${PROJECT_NAME} PUBLIC ${WARC_SOFT_DIR} )

    target_link_libraries( ${PROJECT_NAME} PUBLIC mongo::mongocxx_static PUBLIC mongo::bsoncxx_static PUBLIC mongo::bson_static PUBLIC mongo::mongoc_static )
    target_compile_options( ${PROJECT_NAME} PUBLIC -lmongocxx -lbsoncxx )

    target_link_libraries( ${PROJECT_NAME} PUBLIC IXN )
    target_link_libraries( ${PROJECT_NAME} PUBLIC ImGUI )

    target_include_directories( ${PROJECT_NAME} PUBLIC "C:/Hackerman/mongo-cxx-driver/include/bsoncxx/v_noabi/" )
    target_include_directories( ${PROJECT_NAME} PUBLIC "C:/Hackerman/mongo-cxx-driver/include/mongocxx/v_noabi/" )
endif()