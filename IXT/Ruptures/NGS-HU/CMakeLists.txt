cmake_minimum_required( VERSION 3.30.0 )

project(
    "NGS-HU"
    VERSION 1.0.0
    LANGUAGES C CXX
)

set( HU_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/inc" )
set( HU_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src" )

add_library( "HU" STATIC
    "${HU_SRC_DIR}/inet-ssl.cpp"
)
target_link_libraries( "HU" PUBLIC "IXT" )
target_include_directories( "HU" PUBLIC ${HU_INC_DIR} )

add_executable( ${PROJECT_NAME} "src/main.cpp" )

target_link_libraries( ${PROJECT_NAME} PUBLIC "HU" )
