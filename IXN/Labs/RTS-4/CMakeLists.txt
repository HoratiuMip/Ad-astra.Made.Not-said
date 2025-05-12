cmake_minimum_required( VERSION 3.30.0 )

# add_executable( "RTS-4" main.cpp )
# target_link_libraries( "RTS-4" PUBLIC "IXN" ) 
# target_link_libraries( "RTS-4" PUBLIC "ImGUI" ) 

add_executable( "test_2" main_test_2.cpp )
target_link_libraries( "test_2" PUBLIC "IXN" ) 
target_link_libraries( "test_2" PUBLIC "ImGUI" ) 
