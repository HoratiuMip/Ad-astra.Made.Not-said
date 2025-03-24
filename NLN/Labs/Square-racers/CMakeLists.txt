cmake_minimum_required( VERSION 3.30.0 )


add_executable( square-racers main.cpp )
target_link_libraries( square-racers PUBLIC "NLN" )
target_link_libraries( square-racers PUBLIC "ImGUI" )
