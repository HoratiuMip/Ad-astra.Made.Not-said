cmake_minimum_required( VERSION 3.30.0 )

add_executable( bitmap-fondle main_bit.cpp )

target_link_libraries( bitmap-fondle PUBLIC "IXN" )
