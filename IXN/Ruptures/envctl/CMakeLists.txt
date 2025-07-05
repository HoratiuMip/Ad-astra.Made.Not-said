cmake_minimum_required( VERSION 3.30.0 )

project( envctl )

add_executable( ${PROJECT_NAME} main.cpp )
target_link_libraries( ${PROJECT_NAME} PUBLIC "IXN" )
target_link_libraries( ${PROJECT_NAME} PUBLIC "ImGUI" )
