cmake_minimum_required( VERSION 3.30.0 )


foreach( component ${IXT_COMPONENTS} )
    set( path "${CMAKE_CURRENT_LIST_DIR}/IXT/${component}.hpp.in" )

    if( EXISTS ${path} ) 
        configure_file( ${path} "Configured/IXT/${component}.hpp" )
    endif()

endforeach()

target_include_directories( 
    ${IXT_LIBRARY_NAME} 
    PUBLIC "./" "${CMAKE_CURRENT_BINARY_DIR}/Configured"
)

