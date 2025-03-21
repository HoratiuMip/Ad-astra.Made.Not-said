cmake_minimum_required( VERSION 3.30.0 )

foreach( lab ${NLN_LABS_TO_BUILD} )
    add_subdirectory( "${lab}" )
endforeach()
 