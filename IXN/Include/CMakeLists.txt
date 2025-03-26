cmake_minimum_required( VERSION 3.30.0 )


set( _nln_in_cxx_base_components_include "" )

foreach( component ${IXN_BASE_COMPONENTS} )
    string( APPEND _nln_in_cxx_base_components_include "#include <IXN/${component}.hpp>\n" )
endforeach()

foreach( component ${IXN_BASE_COMPONENTS} )
    set( conf_path "${CMAKE_CURRENT_LIST_DIR}/IXN/${component}.hpp.in" )

    if( EXISTS ${conf_path} ) 
        message( STATUS "Configuring IXN file: ${conf_path}." )
        configure_file( ${conf_path} "Configured/IXN/${component}.hpp" )
    endif()
endforeach()

message( STATUS "IXN file configurations completed." )
message( STATUS "IXN base components includes generated:\n${_nln_in_cxx_base_components_include}" )

target_include_directories( 
    ${IXN_LIB_NAME} 
    PUBLIC "./" "${CMAKE_CURRENT_BINARY_DIR}/Configured"
)
