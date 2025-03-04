cmake_minimum_required( VERSION 3.30.0 )

add_executable( rgba-interlace main.cpp )

target_link_libraries( rgba-interlace PUBLIC "NLN" )
