cmake_minimum_required( VERSION 3.30.0 )

foreach( rup ${IXT_RUPTURES_TO_BUILD} )
    add_subdirectory( "${rup}" )
endforeach()
 