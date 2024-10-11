cmake_minimum_required( VERSION 3.30.0 )


set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )


message( STATUS "Calibrating IXT immersion engine for build..." )


list( LENGTH IXT_GLS sz )
if( NOT ${sz} EQUAL 0 )
    foreach( gl ${IXT_GLS} )
        if( gl STREQUAL "None" )
            option( IXT_GL_NONE ON )
            
            if( NOT ${sz} EQUAL 1 )
                message( ERROR "Declared using both no GLs and GLs: ${IXT_GLS}" )
            endif()

        elseif( gl STREQUAL "OpenGL" )
            option( IXT_GL_OPEN_GL ON )

        elseif( gl STREQUAL "Direct2D1" )
            option( IXT_GL_DIRECT_2D1 ON )

        endif()
    endforeach()
else()
    message( STATUS "Using no GLs." )
endif()


# Quintessentials.
set( IXT_LIB_NAME "IXT" CACHE STRING "IXT lib name." FORCE )
set( IXT_VERSION "v1.0.0" )
set( IXT_VERSION_ALIAS "Frost Slumber" )
set( IXT_VERSION_MAJOR 1 )
set( IXT_VERSION_MINOR 0 )
set( IXT_VERSION_PATCH 0 )


# IXT components. Every component has a .hpp and, possibly, a .cpp file.
set( _ixt_components_ring_0 "" )

file( GLOB rel_file_paths RELATIVE "${CMAKE_CURRENT_LIST_DIR}/Include/IXT" "${CMAKE_CURRENT_LIST_DIR}/Include/IXT/*" )

foreach( rel_file_path ${rel_file_paths} )
    string( REGEX MATCH "^[a-z0-9-]+\.hpp$" rel_file_path_no_config "${rel_file_path}" )
    
    if( NOT rel_file_path_no_config )
        string( REGEX MATCH "^[a-z0-9-]+\.hpp.in$" rel_file_path "${rel_file_path}" )
    else()
        set( rel_file_path ${rel_file_path_no_config} )
    endif()
    
    if( rel_file_path )
        string( REGEX MATCH "^[a-z0-9-]+" rel_file_path "${rel_file_path}" )
        message( STATUS "Detected IXT component: ${rel_file_path}." )
        list( APPEND _ixt_components_ring_0 ${rel_file_path} )
    endif()
endforeach()

message( STATUS "IXT components set." )
set( IXT_COMPONENTS_RING_0 ${_ixt_components_ring_0} CACHE STRING "IXT engine components." FORCE )

# IXT externals.
set(
    IXT_EXT_LIBS
    "-lwinmm"
    "-ld2d1"
    "-lole32"
    "-lComdlg32"
    "-lDwrite"
    "-lwindowscodecs"
    "-lopengl32"
    "-lglu32"
    "-lgdi32"
    "libcomctl32.a"
    "libcomdlg32.a"
    CACHE STRING "IXT external dependencies." FORCE
)


# IXT fondling.
set( IXT_PROJECT_FONDLE_NAME "IXTFondle" )
set( IXT_FONDLE_DIR "${CMAKE_CURRENT_LIST_DIR}/Fondle" )
set( IXT_FONDLE_ASSETS_DIR "${IXT_FONDLE_DIR}/Assets" )

set( IXT_IN_CXX_ASSETS_PATHS "" )

file( GLOB assets_paths "${IXT_FONDLE_ASSETS_DIR}/*" )

foreach( path ${assets_paths} )
    string( FIND ${path} "/" pos REVERSE )
    math( EXPR pos "${pos} + 1" )
    string( SUBSTRING ${path} ${pos} -1 asset_file )

    string( FIND ${asset_file} "." pos REVERSE )
    string( SUBSTRING ${asset_file} 0 ${pos} asset_name )
    math( EXPR pos "${pos} + 1" )
    string( SUBSTRING ${asset_file} ${pos} -1 asset_ext )

    string( TOUPPER ${asset_name} asset_name )
    string( TOUPPER ${asset_ext} asset_ext )

    string( APPEND IXT_IN_CXX_ASSETS_PATHS "\tstatic std::string_view ASSET_${asset_ext}_${asset_name}_PATH = \"${path}\";\n" )
endforeach()

message( STATUS "IXT fondle assets generated:\n${IXT_IN_CXX_ASSETS_PATHS}" )


# Project details.
project(
    "${IXT_PROJECT_FONDLE_NAME}"
    VERSION 1.0.0
    LANGUAGES C CXX 
)


# Preprocessor defines.
#target_compile_definitions( ${IXT_LIB_NAME} ${IXT_PREPROCESSOR_DEFINITIONS} )


# Set those folders' paths into variables.
set( IXT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include" )
set( IXT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Source" )


# IXT include and source directories.
add_subdirectory( "Ext/OpenGL/glm" )
add_subdirectory( "Ext/OpenGL/glfw" )
add_subdirectory( "Ext/OpenGL/glew/build/cmake" )
add_subdirectory( "Source" )
add_subdirectory( "Include" )


target_link_libraries( ${IXT_LIB_NAME} PUBLIC glm::glm )
target_link_libraries( ${IXT_LIB_NAME} PUBLIC glfw )
target_link_libraries( ${IXT_LIB_NAME} PUBLIC glew_s )

target_include_directories( ${IXT_LIB_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Ext/OpenGL/glew/include" )


# IXT fondles to build.
if( IXT_FONDLES_TO_BUILD STREQUAL "ALL" ) 
    file( GLOB files_paths "${IXT_FONDLE_DIR}/*" )
    
    foreach( file_path ${files_paths} )
        string( REGEX MATCH "fdl-[a-z0-9_-]+.cpp$" file_path ${file_path} )
        
        if( file_path )
            list( APPEND IXT_FONDLES_TO_BUILD ${file_path} )
        endif()

    endforeach()
    
    set( _ftb_are_source_files ON )
else()
    set( _ftb_are_source_files OFF )
endif()

foreach( source ${IXT_FONDLES_TO_BUILD} )
    if( _ftb_are_source_files )
        string( REGEX MATCH "^fdl-[a-z0-9_-]+" exe ${source} )
    else()
        set( exe "fdl-${source}" )
        set( source "${exe}.cpp" )
    endif()
    
    if( EXISTS "${IXT_FONDLE_DIR}/${source}" )
        add_executable( ${exe} "${IXT_FONDLE_DIR}/${source}" )
        
        target_link_libraries( ${exe} PUBLIC ${IXT_LIB_NAME} )
        
        target_include_directories( ${exe} PUBLIC ${IXT_INCLUDE_DIR} )
    endif()

endforeach()


# Assembles
add_subdirectory( "Assembles" )
