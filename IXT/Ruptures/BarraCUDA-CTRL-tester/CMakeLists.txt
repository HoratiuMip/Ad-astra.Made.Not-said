cmake_minimum_required( VERSION 3.30.0 )

add_executable( barracuda-ctrl-tester main.cpp )

target_link_libraries( barracuda-ctrl-tester PUBLIC "IXT" )
