cmake_minimum_required( VERSION 3.30.0 )

project(
    A113
    VERSION 1.0.0
    LANGUAGES C CXX
)
set( A113_VERSION_MAJOR 1 )
set( A113_VERSION_MINOR 0 )
set( A113_VERSION_PATCH 0 )
set( A113_VERSION_STRING "auto-a113v1.0.0" )

set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_EXTENSIONS ON )

set( A113_GENERATED_FILE_CAUTION_MSG "/*\n[A113] CAUTION!\nTHIS FILE WAS GENERATED DURING BUILD AND IT WILL BE OVERRIDEN IN THE NEXT ONE.\nDO NOT MODIFY AS THE MODIFICATIONS WILL BE LOST.\n*/")

option( A113_Opt_TargetPlate_OS OFF )
option( A113_Opt_TargetOS_Windows OFF )

option( A113_Opt_TargetPlate_uC OFF )

if( NOT DEFINED A113_TARGET_PLATE )
    message( ERROR "[A113] Target plate not specified. Aborted." )
    return()
endif()

if( A113_TARGET_PLATE STREQUAL "OS" )
    set( A113_Opt_TargetPlate_OS ON )

    if( NOT DEFINED A113_TARGET_OS )
        message( ERROR "[A113] Target operating system not specified. Aborted." )
        return()
    endif()

    if( A113_TARGET_OS STREQUAL "Windows" )
        set( A113_Opt_TargetOS_Windows ON )
    else()
        message( ERROR "[A113] Target operating system \"${A113_TARGET_OS}\" is not valid." )
        return()
    endif()

elseif( A113_TARGET_PLATE STREQUAL "uC" )
    set( A113_Opt_TargetPlate_uC ON )
else()
    message( ERROR "[A113] Target plate \"${A113_TARGET_PLATE}\" is not valid." )
    return()
endif()


include( FetchContent )

function( A113_Fnc_CloneRepo url tag alias chain_dir )
  message( STATUS "[A113] Cloning repository \"${url}\", tag \"${tag}\" to \"${chain_dir}/${alias}\"." )
  FetchContent_Declare(
    ${alias}
    GIT_REPOSITORY "${url}"
    SOURCE_DIR "${chain_dir}/${alias}"
    GIT_TAG "${tag}"
  )
  FetchContent_MakeAvailable( ${alias} )
endfunction( A113_Fnc_CloneRepo )


message( STATUS "[A113] Adding the bedrock plate subdirectory." )
add_subdirectory( "brp" )

if( A113_Opt_TargetPlate_uC )
    message( STATUS "[A113] Adding the micro-controller plate subdirectory." )
    add_subdirectory( "ucp" )
endif()

if( A113_Opt_TargetPlate_OS )
    message( STATUS "[A113] Adding the operating-system plate subdirectory." )
    add_subdirectory( "osp" )
endif()


foreach( framework ${A113_MAKE_FRAMEWORKS} )
    add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/fwork/${framework}" )
endforeach()

foreach( rupture ${A113_MAKE_RUPTURES} )
    add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/Ruptures/${rupture}" )
endforeach()


add_executable( "main" "${CMAKE_CURRENT_LIST_DIR}/main.cpp" )
target_link_libraries( main 
    PUBLIC A113_BRp
    PUBLIC A113_OSp
    PUBLIC A113_Fwork_immersive
)
