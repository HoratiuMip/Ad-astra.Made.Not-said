#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

namespace _ENGINE_NAMESPACE {



inline static struct {
#if defined( _ENGINE_OS_WINDOWS )
    WSADATA    wsa_data;
#endif

} GME_visible_internal;



enum INIT_FLAG : DWORD {
    INIT_FLAG_UPLINK_NETWORK                   = 1 << 0,
    INIT_FLAG_UPLINK_NETWORK_CONTINUE_IF_FAULT = 1 << 1,

    _INIT_FLAG_FORCE_DWORD = 0x7F'FF'FF'FF
};

inline DWORD initial_uplink( int argc, char* argv[], DWORD flags, void* arg, void** ret ) {
    comms() << "Uplinking " _ENGINE_STR " engine...";

#if defined( _ENGINE_OS_WINDOWS )
    if( flags & INIT_FLAG_UPLINK_NETWORK ) {
        comms() << "WSA begin init.";
        memset( &GME_visible_internal.wsa_data, 0, sizeof( WSADATA ) );
        if( WSAStartup( MAKEWORD( 2, 2 ), &GME_visible_internal.wsa_data ) != 0 ) {
            const char* msg = "Fault when starting WSA.";
            if( flags & INIT_FLAG_UPLINK_NETWORK_CONTINUE_IF_FAULT ) {
                comms( ECHO_LEVEL_WARNING ) << msg << " Continuing...";
            } else {
                comms( ECHO_LEVEL_ERROR ) << msg << " Aborting.";
                return -1;
            }
        }
    }
    comms( ECHO_LEVEL_OK ) << "WSA init complete.";
    
#endif

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

inline DWORD final_downlink( int argc, char* argv[], DWORD flags, void* arg, void** ret ) {
    comms() << "Downlinking " _ENGINE_STR " engine...";

#if defined( _ENGINE_GL_OPEN_GL )
    glfwTerminate();
#endif 

    comms( ECHO_LEVEL_OK ) << _ENGINE_STR " downlink complete. Seeya next time.\n";
    return 0;
}



};
