cmake_minimum_required( VERSION 3.30.0 )

add_executable( "RTS-4" main.cpp )
target_link_libraries( "RTS-4" PUBLIC "IXN" ) 
target_link_libraries( "RTS-4" PUBLIC "ImGUI" ) 
