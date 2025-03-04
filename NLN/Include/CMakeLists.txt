cmake_minimum_required( VERSION 3.30.0 )


set( _nln_in_cxx_base_components_include "" )

foreach( component ${NLN_BASE_COMPONENTS} )
    string( APPEND _nln_in_cxx_base_components_include "#include <NLN/${component}.hpp>\n" )
endforeach()

foreach( component ${NLN_BASE_COMPONENTS} )
    set( conf_path "${CMAKE_CURRENT_LIST_DIR}/NLN/${component}.hpp.in" )

    if( EXISTS ${conf_path} ) 
        message( STATUS "Configuring NLN file: ${conf_path}." )
        configure_file( ${conf_path} "Configured/NLN/${component}.hpp" )
    endif()
endforeach()

message( STATUS "NLN file configurations completed." )
message( STATUS "NLN base components includes generated:\n${_nln_in_cxx_base_components_include}" )

target_include_directories( 
    ${NLN_LIB_NAME} 
    PUBLIC "./" "${CMAKE_CURRENT_BINARY_DIR}/Configured"
)
