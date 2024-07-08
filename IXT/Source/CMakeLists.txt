cmake_minimum_required( VERSION 3.30.0 )


# Create the library from all the source files.
set( sources "" )

foreach( component ${IXT_COMPONENTS} )
    set( path "${CMAKE_CURRENT_LIST_DIR}/${component}.cpp" )
    
    if( EXISTS ${path} )
        list( APPEND sources "${component}.cpp" )
    endif()

endforeach()

add_library( ${IXT_LIBRARY_NAME} STATIC ${sources} )
