cmake_minimum_required( VERSION 3.30.0 )

add_executable( blur-tool main.cpp )

target_link_libraries( blur-tool PUBLIC "IXTLib" )
