cmake_minimum_required( VERSION 3.30.0 )

add_executable( "II-2" main.cpp )
target_link_libraries( "II-2" PUBLIC "NLN" ) 
target_link_libraries( "II-2" PUBLIC "ImGUI" )