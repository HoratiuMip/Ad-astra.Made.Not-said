cmake_minimum_required( VERSION 3.30.0 )


set( IXT_IN_CXX_RING_0_FULL_INCLUDES "" )

foreach( component ${IXT_COMPONENTS_RING_0} )
    set( conf_path "${CMAKE_CURRENT_LIST_DIR}/IXT/${component}.hpp.in" )

    string( APPEND IXT_IN_CXX_RING_0_FULL_INCLUDES "#include <IXT/${component}.hpp>\n" )

    if( EXISTS ${conf_path} ) 
        message( STATUS "Configuring IXT file: ${conf_path}." )
        configure_file( ${conf_path} "Configured/IXT/${component}.hpp" )
    endif()

endforeach()

message( STATUS "IXT file configurations completed." )
message( STATUS "IXT ring 0 includes generated:\n${IXT_IN_CXX_RING_0_FULL_INCLUDES}" )

target_include_directories( 
    ${IXT_LIB_NAME} 
    PUBLIC "./" "${CMAKE_CURRENT_BINARY_DIR}/Configured"
)
