cmake_minimum_required( VERSION 3.30.0 )


add_executable( brute-pass main.cpp )
target_link_libraries( brute-pass PUBLIC "IXN" )
target_link_libraries( brute-pass PUBLIC "ImGUI" )
