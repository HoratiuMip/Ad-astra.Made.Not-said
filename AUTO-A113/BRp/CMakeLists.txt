cmake_minimum_required( VERSION 3.30.0 )


set( _chain_dir "${CMAKE_CURRENT_LIST_DIR}/chain" )

configure_file( "${CMAKE_CURRENT_LIST_DIR}/include/A113/BRp/descriptor.hpp.in" "${CMAKE_CURRENT_LIST_DIR}/include/A113/BRp/descriptor.hpp" )

file( GLOB _source_files "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" )

add_library( A113_BRp STATIC ${_source_files} )

target_include_directories( A113_BRp PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" )
if( A113_WJPv3_ROOT_DIR )
    message( STATUS "[A113] Including WJPv3 root directory \"${A113_WJPv3_ROOT_DIR}\"." )
    target_include_directories( A113_BRp PUBLIC "${A113_WJPv3_ROOT_DIR}" )
endif()
