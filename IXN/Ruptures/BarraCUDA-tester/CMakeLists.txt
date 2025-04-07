cmake_minimum_required( VERSION 3.30.0 )


if( IXN_IMGUI )
    add_executable( barracuda-tester-v2 main-v2.cpp )
    target_link_libraries( barracuda-tester-v2 PUBLIC "IXN" )
    target_link_libraries( barracuda-tester-v2 PUBLIC "ImGUI" )
else()
    add_executable( barracuda-tester main.cpp )
    target_link_libraries( barracuda-tester PUBLIC "IXN" )
endif()
