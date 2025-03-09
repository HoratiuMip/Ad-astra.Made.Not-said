cmake_minimum_required( VERSION 3.30.0 )


if( NLN_IMGUI )
    add_executable( barracuda-ctrl-tester-v2 main-v2.cpp )
    target_link_libraries( barracuda-ctrl-tester-v2 PUBLIC "NLN" )
    target_link_libraries( barracuda-ctrl-tester-v2 PUBLIC "ImGUI" )
else()
    add_executable( barracuda-ctrl-tester main.cpp )
    target_link_libraries( barracuda-ctrl-tester PUBLIC "NLN" )
endif()
