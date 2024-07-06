# ------------ # Start-up

# CMake minimum version.
cmake_minimum_required( VERSION 3.30.0 )


# CXX details.
set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )


# IXT components. Every component has a ".hpp" and, optionally, a ".cpp" file. 
set(
    IXT_COMPONENTS
    "bit_manip" "comms" "concepts" "descriptor" "endec"
    "file_manip" "os"
)


# ------------ # PROJECT IXTFondle
set( IXT_PROJECT_FONDLE_NAME "IXTFondle" )

# Project details.
project(
    "${IXT_PROJECT_FONDLE_NAME}"
    VERSION 1.0.0
    LANGUAGES C CXX 
)


# Compile options:
#   -fpermissive : We know what we do.
add_compile_options( "-fpermissive" )

# Preprocessor defines.
add_compile_definitions(
    IXT_OS_WINDOWS
    IXT_GL_NONE
)


# Set those folders' paths into variables.
set( IXT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include" )
set( IXT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Source" )


# IXT include and source directories.
add_subdirectory( "Source" )
add_subdirectory( "Include" )


# All executables of this project.
set( EXES "comms" "endec" )

# Get the libraries names set by the Source/CMakeLists.txt.
get_property( libs_names GLOBAL PROPERTY IXT_LIBRARIES_NAMES )

foreach( exe ${EXES} )
    # Add each executable and its sources/includes.
    add_executable( ${exe} "Fondle/fdl_${exe}.cpp" )

    target_link_libraries( ${exe} PUBLIC ${libs_names} )
    
    target_include_directories( ${exe} PUBLIC ${IXT_INCLUDE_DIR} )
endforeach()
