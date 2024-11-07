cmake_minimum_required( VERSION 3.30.0 )

project(
    "Winter-Arc"
    VERSION 1.0.0
    LANGUAGES C CXX
)

set( WINTER_ARC_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/inc" )
set( WINTER_ARC_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src" )

add_library( "warc" STATIC
    "${WINTER_ARC_SRC_DIR}/inet-tls.cpp"
)
target_link_libraries( "warc" PUBLIC "IXT" )
target_include_directories( "warc" PUBLIC ${WINTER_ARC_INC_DIR} )

add_executable( ${PROJECT_NAME} "src/main.cpp" )

target_link_libraries( ${PROJECT_NAME} PUBLIC "warc" )