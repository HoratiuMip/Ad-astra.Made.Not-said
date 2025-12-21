cmake_minimum_required( VERSION 3.30.0 )

project(
    COMMod
    VERSION 1.0.0
    LANGUAGES C CXX
)

set( _excom_dir "${CMAKE_CURRENT_LIST_DIR}/excom" )

set( MB_EXAMPLES_ENABLED OFF )
A113_Fnc_CloneRepo( "https://github.com/serhmarch/ModbusLib.git" "v0.4.8" modbus "${_excom_dir}" )
target_include_directories( modbus PUBLIC "${_excom_dir}/modbus/src" )

add_executable( ${PROJECT_NAME} main.cpp )
target_link_libraries( ${PROJECT_NAME} 
    PUBLIC A113_OSp
    PUBLIC A113_ClkWrk_immersive
    PUBLIC modbus
)