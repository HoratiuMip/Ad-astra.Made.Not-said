cmake_minimum_required( VERSION 3.30.0 )

add_executable( lab-III main.cpp )

target_compile_definitions( lab-III PUBLIC "SOURCE_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"" )

target_link_libraries( lab-III PUBLIC "IXT" )
