cmake_minimum_required( VERSION 3.30.0 )

add_executable( "Hamming" main.cpp )
target_link_libraries( "Hamming" PUBLIC "IXN" ) 
target_link_libraries( "Hamming" PUBLIC "ImGUI" )