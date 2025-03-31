cmake_minimum_required( VERSION 3.30.0 )


set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )


message( STATUS "Calibrating IXN engine for build..." )


foreach( opt ${IXN_HIGH} )
    set( IXN_${opt} ON )
endforeach()


# Intrinsics filter. ----------------------------------
option( IXN_AVX "IXN_AVX" OFF )


# OS filter. ----------------------------------
option( IXN_OS_WINDOWS "IXN_OS_WINDOWS" OFF )


# GL filter. ----------------------------------
option( IXN_GL_NONE "IXN_GL_NONE" OFF )
option( IXN_GL_OPEN_GL "IXN_GL_OPEN_GL" OFF )
option( IXN_GL_DIRECT_2D1 "IXN_GL_DIRECT_2D1" OFF )


# BOOST filter.  ----------------------------------
option( IXN_BOOST "IXN_BOOST" OFF )


# OpenSSL filter.  ----------------------------------
option( IXN_OPEN_SSL "IXN_OPEN_SSL" OFF )


# ImGUI filter. ----------------------------------
option( IXN_IMGUI "IXN_IMGUI" OFF )


# Quintessentials. ----------------------------------
set( IXN_LIB_NAME "IXN" )
set( IXN_VERSION "v1.0.0" )
set( IXN_VERSION_ALIAS "Frost Slumber" )
set( IXN_VERSION_MAJOR 1 )
set( IXN_VERSION_MINOR 0 )
set( IXN_VERSION_PATCH 0 )


# IXN base components. ----------------------------------
set( _nln_base_components "" )

file( GLOB rel_file_paths RELATIVE "${CMAKE_CURRENT_LIST_DIR}/Include/IXN" "${CMAKE_CURRENT_LIST_DIR}/Include/IXN/*" )

foreach( rel_file_path ${rel_file_paths} )
    string( REGEX MATCH "^[a-z0-9-]+\.hpp$" rel_file_path_no_config "${rel_file_path}" )
    
    if( NOT rel_file_path_no_config )
        string( REGEX MATCH "^[a-z0-9-]+\.hpp.in$" rel_file_path "${rel_file_path}" )
    else()
        set( rel_file_path ${rel_file_path_no_config} )
    endif()
    
    if( rel_file_path )
        string( REGEX MATCH "^[a-z0-9-]+" rel_file_path "${rel_file_path}" )
        message( STATUS "Detected IXN base component: ${rel_file_path}." )
        list( APPEND _nln_base_components ${rel_file_path} )
    endif()
endforeach()

message( STATUS "Setting IXN base components..." )
set( IXN_BASE_COMPONENTS ${_nln_base_components} CACHE STRING "IXN engine base components." FORCE )


# IXN externals. ----------------------------------
set(
    IXN_EXT_LIBS
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
    CACHE STRING "IXN external dependencies." FORCE
)


# IXN fondling. ----------------------------------
set( IXN_PROJECT_FONDLE_NAME "IXNFondle" )
set( IXN_FONDLE_DIR "${CMAKE_CURRENT_LIST_DIR}/Fondle" )
set( IXN_FONDLE_ASSETS_DIR "${IXN_FONDLE_DIR}/Assets" )
set( IXN_EXCOM_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/Excom" )

set( IXN_IN_CXX_ASSETS_PATHS "" )

file( GLOB assets_paths "${IXN_FONDLE_ASSETS_DIR}/*" )

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

    string( APPEND IXN_IN_CXX_ASSETS_PATHS "\tstatic std::string_view ASSET_${asset_ext}_${asset_name}_PATH = \"${path}\";\n" )
endforeach()

message( STATUS "IXN fondle assets generated:\n${IXN_IN_CXX_ASSETS_PATHS}" )


# Project details. ----------------------------------
project(
    "${IXN_PROJECT_FONDLE_NAME}"
    VERSION 1.0.0
    LANGUAGES C CXX 
)

add_compile_options( "-O3" "-march=native" )


# Intrinsics pre. ----------------------------------
if( IXN_AVX )
    add_compile_definitions( "IXN_AVX=${IXN_AVX}" )
endif()


# OS pre. ----------------------------------
if( IXN_OS_WINDOWS )
    message( STATUS "Preparing OS Windows." )

    add_compile_definitions( "IXN_OS_WINDOWS" )
endif()

# GL pre. ----------------------------------
if( IXN_GL_NONE )
    add_compile_definitions( "IXN_GL_NONE" )
endif()

if( IXN_GL_OPEN_GL )
    message( STATUS "Preparing GL OpenGL." )

    option( GLM_BUILD_TESTS OFF )

    add_compile_definitions( "GLM_ENABLE_EXPERIMENTAL" )

    add_subdirectory( "${IXN_EXCOM_ROOT_DIR}/OpenGL/glm" )
    add_subdirectory( "${IXN_EXCOM_ROOT_DIR}/OpenGL/glfw" )
    add_subdirectory( "${IXN_EXCOM_ROOT_DIR}/OpenGL/glew/build/cmake" )
    add_subdirectory( "${IXN_EXCOM_ROOT_DIR}/OpenGL/globj" )

    add_compile_definitions( "IXN_GL_OPEN_GL" )
endif()

if( IXN_GL_DIRECT_2D1 )
    message( STATUS "Preparing GL Direct2D1." )

    add_compile_definitions( "IXN_GL_DIRECT_2D1" )
endif()


# OpenSSL pre. ----------------------------------
set( _nln_open_ssl_libs "" )
if( IXN_OPEN_SSL )
    message( STATUS "Preparing OpenSSL." )

    include_directories( "${IXN_EXCOM_ROOT_DIR}/OpenSSL/include" )
    set( _nln_open_ssl_libs 
        "${IXN_EXCOM_ROOT_DIR}/OpenSSL/libssl.a"
        "${IXN_EXCOM_ROOT_DIR}/OpenSSL/libcrypto.a"
    )
    set( IXN_OPEN_SSL_LIBS ${_nln_open_ssl_libs} CACHE STRING "IXN configurated OpenSSL libraries." FORCE )
    # link_libraries( ${IXN_OPEN_SSL_LIBS} )
endif()

# Boost pre. ----------------------------------
if( IXN_BOOST )
    message( STATUS "Preparing Boost." )

    include_directories( "${IXN_EXCOM_ROOT_DIR}/Boost" )
    link_libraries( "${IXN_EXCOM_ROOT_DIR}/Boost/stage/lib/libboost_json-mgw11-mt-x64-1_86.a" )
endif()


# ImGUI pre. ----------------------------------
if( IXN_IMGUI ) 
    set( _imgui_source_files "" )
    file( GLOB files_paths "${IXN_EXCOM_ROOT_DIR}/ImGUI/*" )
    
    foreach( file_path ${files_paths} )
        string( REGEX MATCH "[a-zA-Z_0-9]+\.cpp$" file_path ${file_path} )
        
        if( file_path )
            message( STATUS "Detected ImGUI source file: ${file_path}" )
            list( APPEND _imgui_source_files "${IXN_EXCOM_ROOT_DIR}/ImGUI/${file_path}" )
        endif()
    endforeach()

    message( STATUS "Creating the ImGUI library..." )
    include_directories( "${IXN_EXCOM_ROOT_DIR}/ImGUI/" )
    include_directories( "${IXN_EXCOM_ROOT_DIR}/ImGUI/backends/" )
    add_library( "ImGUI" STATIC ${_imgui_source_files} )
endif()


# Set those folders' paths into variables. ----------------------------------
set( IXN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Include" )
set( IXN_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Source" )

# IXN include and source directories. ----------------------------------
add_subdirectory( "Source" )
add_subdirectory( "Include" )


# GL post. ----------------------------------
if( IXN_GL_OPEN_GL )
    target_link_libraries( ${IXN_LIB_NAME} PUBLIC glm )
    target_link_libraries( ${IXN_LIB_NAME} PUBLIC glfw )
    target_link_libraries( ${IXN_LIB_NAME} PUBLIC glew_s )   
    target_link_libraries( ${IXN_LIB_NAME} PUBLIC globj )
    
    target_include_directories( ${IXN_LIB_NAME} PUBLIC "${IXN_EXCOM_ROOT_DIR}/OpenGL/glm" )
    target_include_directories( ${IXN_LIB_NAME} PUBLIC "${IXN_EXCOM_ROOT_DIR}/OpenGL/glfw/include" )
    target_include_directories( ${IXN_LIB_NAME} PUBLIC "${IXN_EXCOM_ROOT_DIR}/OpenGL/glew/include" )
endif()

# OpenSSL post. ----------------------------------
if( IXN_OPEN_SSL )
    target_link_libraries( ${IXN_LIB_NAME} PUBLIC ${_nln_open_ssl_libs} )
endif()


# ImGUI post. ----------------------------------
if( IXN_IMGUI )
    if( IXN_GL_OPEN_GL ) 
        target_link_libraries( "ImGUI" PUBLIC glm )
        target_link_libraries( "ImGUI" PUBLIC glfw )
        target_link_libraries( "ImGUI" PUBLIC glew_s )   
        target_link_libraries( "ImGUI" PUBLIC globj )
        
        target_include_directories( "ImGUI" PUBLIC "${IXN_EXCOM_ROOT_DIR}/OpenGL/glm" )
        target_include_directories( "ImGUI" PUBLIC "${IXN_EXCOM_ROOT_DIR}/OpenGL/glfw/include" )
        target_include_directories( "ImGUI" PUBLIC "${IXN_EXCOM_ROOT_DIR}/OpenGL/glew/include" )
    endif()
endif()


# IXN fondles to build. ----------------------------------
if( IXN_FONDLES_TO_BUILD STREQUAL "ALL" ) 
    file( GLOB files_paths "${IXN_FONDLE_DIR}/*" )
    
    foreach( file_path ${files_paths} )
        string( REGEX MATCH "fdl-[a-z0-9_-]+.cpp$" file_path ${file_path} )
        
        if( file_path )
            list( APPEND IXN_FONDLES_TO_BUILD ${file_path} )
        endif()

    endforeach()
    
    set( _ftb_are_source_files ON )
else()
    set( _ftb_are_source_files OFF )
endif()

foreach( source ${IXN_FONDLES_TO_BUILD} )
    if( _ftb_are_source_files )
        string( REGEX MATCH "^fdl-[a-z0-9_-]+" exe ${source} )
    else()
        set( exe "fdl-${source}" )
        set( source "${exe}.cpp" )
    endif()
    
    if( EXISTS "${IXN_FONDLE_DIR}/${source}" )
        add_executable( ${exe} "${IXN_FONDLE_DIR}/${source}" )
        
        target_link_libraries( ${exe} PUBLIC ${IXN_LIB_NAME} )
        
        target_include_directories( ${exe} PUBLIC ${IXN_INCLUDE_DIR} )
    endif()

endforeach()


# Ruptures ----------------------------------
add_subdirectory( "Ruptures" )

# Labs ----------------------------------
add_subdirectory( "Labs" )
