#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

namespace _ENGINE_NAMESPACE {



DWORD initial_uplink( int argc, char* argv[], DWORD flags, void* arg, void** ret ) {
    comms() << "Uplinking " _ENGINE_STR " engine...";

#if defined( _ENGINE_GL_OPEN_GL )
    comms() << "OpenGL GLFW begin init.";
    if( !glfwInit() ) {
        comms( ECHO_LEVEL_ERROR ) << "OpenGL GLFW init fault.";
        return -1;
    }
    comms( ECHO_LEVEL_OK ) << "OpenGL GLFW init complete.";

#endif

    comms( ECHO_LEVEL_OK ) << _ENGINE_STR " engine uplink complete.\n";
    return 0;
}

DWORD final_downlink( int argc, char* argv[], DWORD flags, void* arg, void** ret ) {
    comms() << "Downlinking " _ENGINE_STR " engine...";

#if defined( _ENGINE_GL_OPEN_GL )
    glfwTerminate();
#endif 

    comms( ECHO_LEVEL_OK ) << _ENGINE_STR " downlink complete. Seeya next time.\n";
    return 0;
}



};
