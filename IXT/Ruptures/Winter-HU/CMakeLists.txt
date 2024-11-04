cmake_minimum_required( VERSION 3.30.0 )

project(
    "Winter-HU"
    VERSION 1.0.0
    LANGUAGES C CXX
)

set( WNT_HU_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/inc" )
set( WNT_HU_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src" )

add_library( "WNT-HU" STATIC
    "${WNT_HU_SRC_DIR}/inet-tls.cpp"
)
target_link_libraries( "WNT-HU" PUBLIC "IXT" )
target_include_directories( "WNT-HU" PUBLIC ${WNT_HU_INC_DIR} )

add_executable( ${PROJECT_NAME} "src/main.cpp" )

target_link_libraries( ${PROJECT_NAME} PUBLIC "WNT-HU" )
