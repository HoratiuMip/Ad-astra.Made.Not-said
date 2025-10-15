#pragma once
/**
 * @file: OSp/core.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <atomic>
#include <functional>

#ifdef A113_TARGET_OS_WINDOWS
    #include <winsock2.h>
    #include <ws2spi.h>
    #include <windows.h>
    #include <wincodec.h>
    #include <Ws2bth.h>
    #include <BluetoothAPIs.h>
#endif

namespace A113 { namespace OSp {


inline std::shared_ptr< spdlog::logger >   Log   = nullptr;


class INTERNAL {
A113_PROTECTED:
    struct {
    #ifdef A113_TARGET_OS_WINDOWS
        WSADATA   wsa_data;
    #endif

    } _Data;

public:
    enum InitFlags_ {
        InitFlag_None = 0x0,
        InitFlag_Sockets
    };
    struct init_args_t {
        int   flags   = InitFlag_None;
    };
    RESULT init( int argc, char* argv[], const init_args_t& args );

}; inline INTERNAL Internal;

struct ON_FNC_EXIT {
    typedef   std::function< void() >   proc_t;
    ON_FNC_EXIT( proc_t proc ) : _proc{ ( proc_t&& )proc } {}
    std::function< void() >   _proc;
    ~ON_FNC_EXIT() { if( nullptr != _proc ) _proc(); } 
    A113_inline void drop( void ) { _proc = nullptr; }
};


} };



