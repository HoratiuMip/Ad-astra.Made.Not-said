cmake_minimum_required( VERSION 3.30.0 )

add_executable( lab-IV main.cpp )

target_compile_definitions( lab-IV PUBLIC "SOURCE_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"" )

target_link_libraries( lab-IV PUBLIC "IXT" ) 
 