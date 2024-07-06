# CMake minimum version.
cmake_minimum_required( VERSION 3.30.0 )


# IXT libraries list.
set( libs_list "" )

# Iterate through ITX components.
foreach( component ${IXT_COMPONENTS} )
    # Set the path to source and lib's name.
    set( path "${CMAKE_CURRENT_LIST_DIR}/${component}.cpp" )
    set( lib "lib_${component}")
    

    if( EXISTS ${path} )
        # If the path exists, add that lib as STATIC and its include dir.
        add_library( ${lib} STATIC ${path} )

        target_include_directories( ${lib} PUBLIC ${IXT_INCLUDE_DIR} )
    else()
        # Else add that lib as INTERFACE a.k.a header only.
        add_library( ${lib} INTERFACE )
    endif()

    # Append component's lib to the list.
    list( APPEND libs_list ${lib} )

endforeach()

set_property( GLOBAL PROPERTY IXT_LIBRARIES_NAMES ${libs_list} )
