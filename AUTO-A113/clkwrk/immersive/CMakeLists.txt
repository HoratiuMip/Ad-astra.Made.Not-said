cmake_minimum_required( VERSION 3.30.0 )

project(
    A113_ClkWrk_immersive
    VERSION 1.0.0
    LANGUAGES C CXX
)

file( GLOB _source_files "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" )
add_library( ${PROJECT_NAME} STATIC ${_source_files} )
target_link_libraries( ${PROJECT_NAME} 
    PUBLIC A113_BRp
    PUBLIC A113_OSp 
)
target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" )
