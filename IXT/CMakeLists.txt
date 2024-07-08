cmake_minimum_required( VERSION 3.30.0 )


set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )


list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake" )
include( "IXTDescriptor" )



# ------------ # PROJECT IXTFondle
set( IXT_PROJECT_FONDLE_NAME "IXTFondle" )

# Project details.
project(
    "${IXT_PROJECT_FONDLE_NAME}"
    VERSION 1.0.0
    LANGUAGES C CXX 
)


# Preprocessor defines.
add_compile_definitions(
    IXT_OS_WINDOWS
    IXT_GL_NONE
)


# Set those folders' paths into variables.
set( IXT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include/IXT" )
set( IXT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Source" )


# IXT include and source directories.
add_subdirectory( "Source" )
add_subdirectory( "Include" )


# All executables of this project.
set( EXES "comms" "endec" )

foreach( exe ${EXES} )
    # Add each executable and its sources/includes.
    add_executable( ${exe} "Fondle/fdl_${exe}.cpp" )

    target_link_libraries( ${exe} PUBLIC ${IXT_LIBRARY_NAME} )
    
    target_include_directories( ${exe} PUBLIC ${IXT_INCLUDE_DIR} )
endforeach()
