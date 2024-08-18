cmake_minimum_required( VERSION 3.30.0 )


set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )


cmake_minimum_required( VERSION 3.30.0 )


# Name of IXT library.
set( IXT_LIB_NAME "IXTLib" CACHE STRING "IXT lib name." FORCE )


# IXT components. Every component has a .hpp and, possibly, a .cpp file.
set(
    IXT_COMPONENTS
    "aritm" 
    "audio" 
    "bit_manip" 
    "comms" 
    "concepts" 
    "descriptor" 
    "endec"
    "env"
    "file_manip" 
    "os" 
    "render2" 
    "surface" 
    "volatile_ptr"
    CACHE STRING "IXT engine components." FORCE
)


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


# ------------ # PROJECT IXTFondle
set( IXT_PROJECT_FONDLE_NAME "IXTFondle" )

set( IXT_VERSION "v1.0.0" )
set( IXT_VERSION_ALIAS "Frost Slumber" )
set( IXT_VERSION_MAJOR 1 )
set( IXT_VERSION_MINOR 0 )
set( IXT_VERSION_PATCH 0 )


# IXT fondling.
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


# Project details.
project(
    "${IXT_PROJECT_FONDLE_NAME}"
    VERSION 1.0.0
    LANGUAGES C CXX 
)


# Preprocessor defines.
#target_compile_definitions( ${IXT_LIB_NAME} ${IXT_PREPROCESSOR_DEFS} )


# Set those folders' paths into variables.
set( IXT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include" )
set( IXT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Source" )


# IXT include and source directories.
add_subdirectory( "Source" )
add_subdirectory( "Include" )


# IXT fondles to build.
if( IXT_FONDLES_TO_BUILD STREQUAL "ALL" ) 
    file( GLOB files_paths "${IXT_FONDLE_DIR}/*" )
    
    foreach( file_path ${files_paths} )
        string( REGEX MATCH "fdl_[a-z]+.cpp$" file_path ${file_path} )
        
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
        string( REGEX MATCH "^fdl_[a-z]+" exe ${source} )
    else()
        set( exe "fdl_${source}" )
        set( source "${exe}.cpp" )
    endif()
    
    if( EXISTS "${IXT_FONDLE_DIR}/${source}" )
        add_executable( ${exe} "${IXT_FONDLE_DIR}/${source}" )
        
        target_link_libraries( ${exe} PUBLIC ${IXT_LIB_NAME} )
        
        target_include_directories( ${exe} PUBLIC ${IXT_INCLUDE_DIR} )
    endif()

endforeach()
