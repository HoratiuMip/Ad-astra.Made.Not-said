cmake_minimum_required( VERSION 3.30.0 )


## Chain dependencies.
set( _chain_dir "${CMAKE_CURRENT_LIST_DIR}/chain" )

# Logging
A113_Fnc_CloneRepo( "https://github.com/gabime/spdlog.git" "v1.15.3" spdlog "${_chain_dir}" )
# OpenGL
A113_Fnc_CloneRepo( "https://github.com/g-truc/glm.git" "1.0.1" glm "${_chain_dir}" )
A113_Fnc_CloneRepo( "https://github.com/glfw/glfw.git" "3.4" glfw "${_chain_dir}" )

# JSON
A113_Fnc_CloneRepo( "https://github.com/nlohmann/json.git" "v3.12.0" nlohmann_json "${_chain_dir}")


## Build A113.
file( GLOB _source_files "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" )
add_library( A113_OSp STATIC ${_source_files} )
target_link_libraries( A113_OSp PUBLIC A113_BRp )
target_include_directories( A113_OSp PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" )

if( A113_Opt_TargetOS_Windows )
    target_compile_definitions( A113_OSp PUBLIC A113_TARGET_OS_WINDOWS )

    target_link_libraries( A113_OSp 
        PUBLIC "-lwinmm"
        PUBLIC "-ld2d1"
        PUBLIC "-lole32"
        PUBLIC "-lComdlg32"
        PUBLIC "-lDwrite"
        PUBLIC "-lwindowscodecs"
        PUBLIC "-lopengl32"
        PUBLIC "-lglu32"
        PUBLIC "-lgdi32"
        PUBLIC "-lws2_32"
        PUBLIC "-lwsock32"
        PUBLIC "-lbthprops"
        PUBLIC "-lcrypt32"
        PUBLIC "libcomctl32.a"
        PUBLIC "libcomdlg32.a"
    )
endif()

## Link A113 with the chain dependencies. 
target_link_libraries( A113_OSp
    PUBLIC spdlog
    PUBLIC glm
    PUBLIC glfw
)
target_include_directories( A113_OSp
    PUBLIC "${_chain_dir}/spdlog/include"
    PUBLIC "${_chain_dir}/glm"
    PUBLIC "${_chain_dir}/glfw/include"
)

