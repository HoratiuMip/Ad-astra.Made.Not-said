cmake_minimum_required( VERSION 3.30.0 )


set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )


message( STATUS "Calibrating NLN engine for build..." )


# Intrinsics filter. ----------------------------------
option( NLN_AVX "NLN_AVX" OFF )


# OS filter. ----------------------------------
option( NLN_OS_WINDOWS "NLN_OS_WINDOWS" OFF )


# GL filter. ----------------------------------
option( NLN_GL_NONE "NLN_GL_NONE" OFF )
option( NLN_GL_OPEN_GL "NLN_GL_OPEN_GL" OFF )
option( NLN_GL_DIRECT_2D1 "NLN_GL_DIRECT_2D1" OFF )


# BOOST filter.  ----------------------------------
option( NLN_BOOST "NLN_BOOST" OFF )


# OpenSSL filter.  ----------------------------------
option( NLN_OPEN_SSL "NLN_OPEN_SSL" OFF )


# Quintessentials. ----------------------------------
set( NLN_LIB_NAME "NLN" )
set( NLN_VERSION "v1.0.0" )
set( NLN_VERSION_ALIAS "Frost Slumber" )
set( NLN_VERSION_MAJOR 1 )
set( NLN_VERSION_MINOR 0 )
set( NLN_VERSION_PATCH 0 )


# NLN base components. ----------------------------------
set( _nln_base_components "" )

file( GLOB rel_file_paths RELATIVE "${CMAKE_CURRENT_LIST_DIR}/Include/NLN" "${CMAKE_CURRENT_LIST_DIR}/Include/NLN/*" )

foreach( rel_file_path ${rel_file_paths} )
    string( REGEX MATCH "^[a-z0-9-]+\.hpp$" rel_file_path_no_config "${rel_file_path}" )
    
    if( NOT rel_file_path_no_config )
        string( REGEX MATCH "^[a-z0-9-]+\.hpp.in$" rel_file_path "${rel_file_path}" )
    else()
        set( rel_file_path ${rel_file_path_no_config} )
    endif()
    
    if( rel_file_path )
        string( REGEX MATCH "^[a-z0-9-]+" rel_file_path "${rel_file_path}" )
        message( STATUS "Detected NLN base component: ${rel_file_path}." )
        list( APPEND _nln_base_components ${rel_file_path} )
    endif()
endforeach()

message( STATUS "Setting NLN base components..." )
set( NLN_BASE_COMPONENTS ${_nln_base_components} CACHE STRING "NLN engine base components." FORCE )


# NLN externals. ----------------------------------
set(
    NLN_EXT_LIBS
    "-lwinmm"
    "-ld2d1"
    "-lole32"
    "-lComdlg32"
    "-lDwrite"
    "-lwindowscodecs"
    "-lopengl32"
    "-lglu32"
    "-lgdi32"
    "-lws2_32"
    "-lwsock32"
    "-lbthprops"
    "-lcrypt32"
    "libcomctl32.a"
    "libcomdlg32.a"
    CACHE STRING "NLN external dependencies." FORCE
)


# NLN fondling. ----------------------------------
set( NLN_PROJECT_FONDLE_NAME "NLNFondle" )
set( NLN_FONDLE_DIR "${CMAKE_CURRENT_LIST_DIR}/Fondle" )
set( NLN_FONDLE_ASSETS_DIR "${NLN_FONDLE_DIR}/Assets" )
set( NLN_EXCOM_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/Excom" )

set( NLN_IN_CXX_ASSETS_PATHS "" )

file( GLOB assets_paths "${NLN_FONDLE_ASSETS_DIR}/*" )

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

    string( APPEND NLN_IN_CXX_ASSETS_PATHS "\tstatic std::string_view ASSET_${asset_ext}_${asset_name}_PATH = \"${path}\";\n" )
endforeach()

message( STATUS "NLN fondle assets generated:\n${NLN_IN_CXX_ASSETS_PATHS}" )


# Project details. ----------------------------------
project(
    "${NLN_PROJECT_FONDLE_NAME}"
    VERSION 1.0.0
    LANGUAGES C CXX 
)

add_compile_options( "-O3" "-march=native" )


# Intrinsics pre. ----------------------------------
if( NLN_AVX )
    add_compile_definitions( "NLN_AVX=${NLN_AVX}" )
endif()


# OS pre. ----------------------------------
if( NLN_OS_WINDOWS )
    message( STATUS "Preparing OS Windows." )

    add_compile_definitions( "NLN_OS_WINDOWS" )
endif()

# GL pre. ----------------------------------
if( NLN_GL_NONE )
    add_compile_definitions( "NLN_GL_NONE" )
endif()

if( NLN_GL_OPEN_GL )
    message( STATUS "Preparing GL OpenGL." )

    option( GLM_BUILD_TESTS OFF )

    add_compile_definitions( "GLM_ENABLE_EXPERIMENTAL" )

    add_subdirectory( "${NLN_EXCOM_ROOT_DIR}/OpenGL/glm" )
    add_subdirectory( "${NLN_EXCOM_ROOT_DIR}/OpenGL/glfw" )
    add_subdirectory( "${NLN_EXCOM_ROOT_DIR}/OpenGL/glew/build/cmake" )
    add_subdirectory( "${NLN_EXCOM_ROOT_DIR}/OpenGL/globj" )

    add_compile_definitions( "NLN_GL_OPEN_GL" )
endif()

if( NLN_GL_DIRECT_2D1 )
    message( STATUS "Preparing GL Direct2D1." )

    add_compile_definitions( "NLN_GL_DIRECT_2D1" )
endif()


# OpenSSL pre. ----------------------------------
set( _nln_open_ssl_libs "" )
if( NLN_OPEN_SSL )
    message( STATUS "Preparing OpenSSL." )

    include_directories( "${NLN_EXCOM_ROOT_DIR}/OpenSSL/include" )
    set( _nln_open_ssl_libs 
        "${NLN_EXCOM_ROOT_DIR}/OpenSSL/libssl.a"
        "${NLN_EXCOM_ROOT_DIR}/OpenSSL/libcrypto.a"
    )
    set( NLN_OPEN_SSL_LIBS ${_nln_open_ssl_libs} CACHE STRING "NLN configurated OpenSSL libraries." FORCE )
    # link_libraries( ${NLN_OPEN_SSL_LIBS} )
endif()

# Boost pre. ----------------------------------
if( NLN_BOOST )
    message( STATUS "Preparing Boost." )

    include_directories( "${NLN_EXCOM_ROOT_DIR}/Boost" )
    link_libraries( "${NLN_EXCOM_ROOT_DIR}/Boost/stage/lib/libboost_json-mgw11-mt-x64-1_86.a" )
endif()

# Set those folders' paths into variables. ----------------------------------
set( NLN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include" )
set( NLN_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Source" )


# NLN include and source directories. ----------------------------------
add_subdirectory( "Source" )
add_subdirectory( "Include" )


# GL post. ----------------------------------
if( NLN_GL_OPEN_GL )
    target_link_libraries( ${NLN_LIB_NAME} PUBLIC glm )
    target_link_libraries( ${NLN_LIB_NAME} PUBLIC glfw )
    target_link_libraries( ${NLN_LIB_NAME} PUBLIC glew_s )   
    target_link_libraries( ${NLN_LIB_NAME} PUBLIC globj )
    
    target_include_directories( ${NLN_LIB_NAME} PUBLIC "${NLN_EXCOM_ROOT_DIR}/OpenGL/glm" )
    target_include_directories( ${NLN_LIB_NAME} PUBLIC "${NLN_EXCOM_ROOT_DIR}/OpenGL/glfw/include" )
    target_include_directories( ${NLN_LIB_NAME} PUBLIC "${NLN_EXCOM_ROOT_DIR}/OpenGL/glew/include" )
endif()

# OpenSSL post. ----------------------------------
if( NLN_OPEN_SSL )
    target_link_libraries( ${NLN_LIB_NAME} PUBLIC ${_nln_open_ssl_libs} )
endif()


# NLN fondles to build. ----------------------------------
if( NLN_FONDLES_TO_BUILD STREQUAL "ALL" ) 
    file( GLOB files_paths "${NLN_FONDLE_DIR}/*" )
    
    foreach( file_path ${files_paths} )
        string( REGEX MATCH "fdl-[a-z0-9_-]+.cpp$" file_path ${file_path} )
        
        if( file_path )
            list( APPEND NLN_FONDLES_TO_BUILD ${file_path} )
        endif()

    endforeach()
    
    set( _ftb_are_source_files ON )
else()
    set( _ftb_are_source_files OFF )
endif()

foreach( source ${NLN_FONDLES_TO_BUILD} )
    if( _ftb_are_source_files )
        string( REGEX MATCH "^fdl-[a-z0-9_-]+" exe ${source} )
    else()
        set( exe "fdl-${source}" )
        set( source "${exe}.cpp" )
    endif()
    
    if( EXISTS "${NLN_FONDLE_DIR}/${source}" )
        add_executable( ${exe} "${NLN_FONDLE_DIR}/${source}" )
        
        target_link_libraries( ${exe} PUBLIC ${NLN_LIB_NAME} )
        
        target_include_directories( ${exe} PUBLIC ${NLN_INCLUDE_DIR} )
    endif()

endforeach()


# Ruptures ----------------------------------
add_subdirectory( "Ruptures" )
