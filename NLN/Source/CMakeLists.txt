cmake_minimum_required( VERSION 3.30.0 )

set( sources "" )

foreach( component ${NLN_BASE_COMPONENTS} )
    set( path "${CMAKE_CURRENT_LIST_DIR}/${component}.cpp" )
    
    if( EXISTS ${path} )
        list( APPEND sources "${component}.cpp" )
    endif()

endforeach()

add_library( ${NLN_LIB_NAME} STATIC ${sources} )
target_compile_definitions( ${NLN_LIB_NAME} PUBLIC ${NLN_PREPROCESSOR_DEFINITIONS} )
target_link_libraries( ${NLN_LIB_NAME} PUBLIC ${NLN_EXT_LIBS} )
