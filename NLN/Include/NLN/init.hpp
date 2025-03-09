#pragma once
/*
*/

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>

namespace _ENGINE_NAMESPACE {



inline static struct {
#if defined( _ENGINE_OS_WINDOWS )
    WSADATA    wsa_data;
#endif

} RUNTIME;



enum BEGIN_RUNTIME_FLAG : DWORD {
    BEGIN_RUNTIME_FLAG_INIT_NETWORK                   = 1 << 0,
    BEGIN_RUNTIME_FLAG_INIT_NETWORK_CONTINUE_IF_FAULT = 1 << 1,

    _BEGIN_RUNTIME_FLAG_FORCE_DWORD = 0x7f'ff'ff'ff
};

DWORD begin_runtime( int argc, char* argv[], DWORD flags, void* in, void** out ) {
    comms() << "Beginning " _ENGINE_STR " engine runtime...";
{
    DWORD fault_ctr = 0;

#if defined( _ENGINE_OS_WINDOWS )
    if( flags & BEGIN_RUNTIME_FLAG_INIT_NETWORK ) {
        comms() << "Network WSA init...";
        memset( &RUNTIME.wsa_data, 0, sizeof( WSADATA ) );
        if( WSAStartup( MAKEWORD( 2, 2 ), &RUNTIME.wsa_data ) != 0 ) {
            ++fault_ctr;
            comms( ECHO_LEVEL_ERROR ) << "Network WSA init fault ( " << WSAGetLastError() << " ).";

            if( !( flags & BEGIN_RUNTIME_FLAG_INIT_NETWORK_CONTINUE_IF_FAULT ) ) goto l_begin_abort;
        }
    }
    comms( ECHO_LEVEL_OK ) << "Network WSA init complete.";
#endif

#if defined( _ENGINE_GL_OPEN_GL )
    comms() << "OpenGL GLFW init...";
    if( !glfwInit() ) {
        comms( ECHO_LEVEL_ERROR ) << "OpenGL GLFW init fault.";
        return -1;
    }
    comms( ECHO_LEVEL_OK ) << "OpenGL GLFW init complete.";
#endif

    comms( fault_ctr == 0 ? ECHO_LEVEL_OK : ECHO_LEVEL_WARNING ) << _ENGINE_STR " engine runtime begin complete with ( " << fault_ctr << " ) faults.\n";
    return 0;

} l_begin_abort:
    comms( ECHO_LEVEL_ERROR ) << "Componnet init fault treated as critical, engine runtime begin aborted.";
    return -1;
}

DWORD end_runtime( int argc, char* argv[], DWORD flags, void* arg, void** ret ) {
    comms() << "Ending " _ENGINE_STR " engine runtime...";

    DWORD fault_crt = 0;

#if defined( _ENGINE_OS_WINDOWS )
    comms() << "Network WSA clean...";
    if( WSACleanup() != 0 ) {
        ++fault_crt;
        comms( ECHO_LEVEL_ERROR ) << "Network WSA clean fault ( " << WSAGetLastError() << " ).";
    } else {
        comms( ECHO_LEVEL_OK ) << "Network WSA clean complete.";
    }
#endif

#if defined( _ENGINE_GL_OPEN_GL )
    comms() << "OpenGL GLFW terminate...";
    glfwTerminate();
    comms( ECHO_LEVEL_OK ) << "OpenGL GLFW terminate complete.";
#endif 

    comms( ECHO_LEVEL_OK ) << _ENGINE_STR " runtime end complete. Shutting down.\n";
    return fault_crt;
}



};
