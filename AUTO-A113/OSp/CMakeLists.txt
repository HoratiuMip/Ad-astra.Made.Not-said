cmake_minimum_required( VERSION 3.30.0 )


## Chain dependencies.
set( _chain_dir "${CMAKE_CURRENT_LIST_DIR}/chain" )

# Logging
A113_Fnc_CloneRepo( "https://github.com/gabime/spdlog.git" "v1.15.3" spdlog "${_chain_dir}" )
# OpenGL
A113_Fnc_CloneRepo( "https://github.com/g-truc/glm.git" "1.0.1" glm "${_chain_dir}" )
A113_Fnc_CloneRepo( "https://github.com/glfw/glfw.git" "3.4" glfw "${_chain_dir}" )
A113_Fnc_CloneRepo( "https://github.com/Perlmint/glew-cmake.git" "glew-cmake-2.2.0" glew "${_chain_dir}" )
A113_Fnc_CloneRepo( "https://github.com/nothings/stb.git" "" stb "${_chain_dir}" )
A113_Fnc_CloneRepo( "https://github.com/tinyobjloader/tinyobjloader.git" "v1.0.6" tinyobj "${_chain_dir}" )

# JSON
A113_Fnc_CloneRepo( "https://github.com/nlohmann/json.git" "v3.12.0" nlohmann_json "${_chain_dir}")

# Dear ImGUI
A113_Fnc_CloneRepo( "https://github.com/ocornut/imgui.git" "v1.91.9-docking" imgui "${_chain_dir}" )
A113_Fnc_CloneRepo( "https://github.com/epezent/implot.git" "v0.16" implot "${_chain_dir}" )


file( GLOB _imgui_source_files "${_chain_dir}/imgui/*.cpp" )
add_library( ImGui STATIC 
    ${_imgui_source_files} 
    "${_chain_dir}/imgui/misc/cpp/imgui_stdlib.cpp" 
    "${_chain_dir}/imgui/backends/imgui_impl_opengl3.cpp" 
    "${_chain_dir}/imgui/backends/imgui_impl_glfw.cpp" 
)
target_include_directories( ImGui 
    PUBLIC "${_chain_dir}/imgui/" 
    PUBLIC "${_chain_dir}/imgui/misc/cpp/"
    PUBLIC "${_chain_dir}/imgui/backends/"
    PUBLIC "${_chain_dir}/glfw/include"
)
file( GLOB _implot_source_files "${_chain_dir}/implot/*.cpp" )
add_library( ImPlot STATIC ${_implot_source_files} )
target_include_directories( ImPlot PUBLIC "${_chain_dir}/imgui/" )
target_include_directories( ImPlot PUBLIC "${_chain_dir}/implot/" )


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
    PUBLIC libglew_static
    PUBLIC nlohmann_json
    PUBLIC ImGui
    PUBLIC ImPlot
)
target_include_directories( A113_OSp
    PUBLIC "${_chain_dir}/spdlog/include"
    PUBLIC "${_chain_dir}/glm"
    PUBLIC "${_chain_dir}/glfw/include"
    PUBLIC "${_chain_dir}/glew/include"
    PUBLIC "${_chain_dir}/stb"
    PUBLIC "${_chain_dir}/tinyobj"
    PUBLIC "${_chain_dir}/nlohmann_json/include"
    PUBLIC "${_chain_dir}/imgui"
    PUBLIC "${_chain_dir}/implot"
)


## Utilities
set( _all_includes_str "" )
file( GLOB _include_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}/include/" "${CMAKE_CURRENT_LIST_DIR}/include/a113/osp/*.hpp" )

foreach( include_file ${_include_files} )
    string( APPEND _all_includes_str "#include <${include_file}>\n" )
endforeach()

configure_file( "${CMAKE_CURRENT_LIST_DIR}/include/a113/osp/osp.hpp.in" "${CMAKE_CURRENT_LIST_DIR}/include/a113/osp/osp.hpp" )


add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/fondles" )