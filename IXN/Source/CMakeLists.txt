cmake_minimum_required( VERSION 3.30.0 )

set( sources "" )

foreach( component ${IXN_BASE_COMPONENTS} )
    set( path "${CMAKE_CURRENT_LIST_DIR}/${component}.cpp" )
    
    if( EXISTS ${path} )
        list( APPEND sources "${component}.cpp" )
    endif()

endforeach()

add_library( ${IXN_LIB_NAME} STATIC ${sources} )
target_compile_definitions( ${IXN_LIB_NAME} PUBLIC ${IXN_PREPROCESSOR_DEFINITIONS} )
target_link_libraries( ${IXN_LIB_NAME} PUBLIC ${IXN_EXT_LIBS} )
