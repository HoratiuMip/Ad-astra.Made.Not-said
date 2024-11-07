cmake_minimum_required( VERSION 3.30.0 )

file( GLOB rel_file_paths RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/*.cpp" )

message( STATUS "Lab-VI src files: ${rel_file_paths}" )

add_executable( lab-VI ${rel_file_paths} )

target_compile_definitions( lab-VI PUBLIC "SOURCE_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"" )

target_link_libraries( lab-VI PUBLIC "IXT" ) 
 