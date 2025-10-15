cmake_minimum_required( VERSION 3.30.0 )

file( GLOB _source_files "${CMAKE_CURRENT_LIST_DIR}/*.cpp" )

add_library( A113_uCp STATIC ${_source_files} )

target_link_libraries( A113_uCp PUBLIC A113_BRp )
target_include_directories( A113_uCp PUBLIC "${A113_IncludeRoot}" )
