cmake_minimum_required( VERSION 3.30.0 )


## EXCOM dependencies.
set( _excom_dir "${CMAKE_CURRENT_LIST_DIR}/excom" )

# Logging
A113_Fnc_CloneRepo( "https://github.com/gabime/spdlog.git" "v1.15.3" spdlog "${_excom_dir}" )
# OpenGL
A113_Fnc_CloneRepo( "https://github.com/g-truc/glm.git" "1.0.1" glm "${_excom_dir}" )
A113_Fnc_CloneRepo( "https://github.com/glfw/glfw.git" "3.4" glfw "${_excom_dir}" )
A113_Fnc_CloneRepo( "https://github.com/Perlmint/glew-cmake.git" "glew-cmake-2.2.0" glew "${_excom_dir}" )
A113_Fnc_CloneRepo( "https://github.com/nothings/stb.git" "" stb "${_excom_dir}" )
A113_Fnc_CloneRepo( "https://github.com/tinyobjloader/tinyobjloader.git" "v1.0.6" tinyobj "${_excom_dir}" )

# JSON
A113_Fnc_CloneRepo( "https://github.com/nlohmann/json.git" "v3.12.0" nlohmann_json "${_excom_dir}")

# Dear ImGUI
A113_Fnc_CloneRepo( "https://github.com/ocornut/imgui.git" "v1.91.9-docking" imgui "${_excom_dir}" )
A113_Fnc_CloneRepo( "https://github.com/epezent/implot.git" "v0.16" implot "${_excom_dir}" )
A113_Fnc_JustCloneRepo( "https://github.com/aiekick/ImGuiFileDialog.git" "v0.6.8" imgui_file_dialog "${_excom_dir}" )


file( GLOB _imgui_source_files "${_excom_dir}/imgui/*.cpp" )
add_library( ImGui STATIC 
    ${_imgui_source_files} 
    "${_excom_dir}/imgui/misc/cpp/imgui_stdlib.cpp" 
    "${_excom_dir}/imgui/backends/imgui_impl_opengl3.cpp" 
    "${_excom_dir}/imgui/backends/imgui_impl_glfw.cpp" 
)
target_include_directories( ImGui 
    PUBLIC "${_excom_dir}/imgui/" 
    PUBLIC "${_excom_dir}/imgui/misc/cpp/"
    PUBLIC "${_excom_dir}/imgui/backends/"
    PUBLIC "${_excom_dir}/glfw/include"
)

file( GLOB _implot_source_files "${_excom_dir}/implot/*.cpp" )
add_library( ImPlot STATIC ${_implot_source_files} )
target_include_directories( ImPlot PUBLIC "${_excom_dir}/imgui/" )
target_include_directories( ImPlot PUBLIC "${_excom_dir}/implot/" )

file( GLOB _imgui_file_dialog_source_files "${_excom_dir}/imgui_file_dialog/*.cpp" )
add_library( ImFileDialog STATIC ${_imgui_file_dialog_source_files} )
target_include_directories( ImFileDialog PUBLIC "${_excom_dir}/imgui/" )
target_include_directories( ImFileDialog PUBLIC "${_excom_dir}/imgui_file_dialog/" )

add_library( One_ImGui INTERFACE )
target_link_libraries( One_ImGui INTERFACE
    ImGui
    ImPlot
    ImFileDialog
)


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
        PUBLIC "-lcfgmgr32"
        PUBLIC "libcomctl32.a"
        PUBLIC "libcomdlg32.a"
    )
endif()

## Link A113 with the EXCOM dependencies. 
target_link_libraries( A113_OSp
    PUBLIC spdlog
    PUBLIC glm
    PUBLIC glfw
    PUBLIC libglew_static
    PUBLIC nlohmann_json
    PUBLIC One_ImGui
)
target_include_directories( A113_OSp
    PUBLIC "${_excom_dir}/spdlog/include"
    PUBLIC "${_excom_dir}/glm"
    PUBLIC "${_excom_dir}/glfw/include"
    PUBLIC "${_excom_dir}/glew/include"
    PUBLIC "${_excom_dir}/stb"
    PUBLIC "${_excom_dir}/tinyobj"
    PUBLIC "${_excom_dir}/nlohmann_json/include"
    PUBLIC "${_excom_dir}/imgui"
    PUBLIC "${_excom_dir}/implot"
)


## Utilities
set( _all_includes_str "" )
file( GLOB _include_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}/include/" "${CMAKE_CURRENT_LIST_DIR}/include/a113/osp/*.hpp" )

foreach( include_file ${_include_files} )
    string( APPEND _all_includes_str "#include <${include_file}>\n" )
endforeach()

configure_file( "${CMAKE_CURRENT_LIST_DIR}/include/a113/osp/osp.hpp.in" "${CMAKE_CURRENT_LIST_DIR}/include/a113/osp/osp.hpp" )


add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/fondles" )