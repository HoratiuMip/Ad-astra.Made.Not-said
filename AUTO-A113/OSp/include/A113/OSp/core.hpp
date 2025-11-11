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
#include <filesystem>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>

#ifdef A113_TARGET_OS_WINDOWS
    #include <winsock2.h>
    #include <ws2spi.h>
    #include <windows.h>
    #include <wincodec.h>
    #include <Ws2bth.h>
    #include <BluetoothAPIs.h>
#endif

namespace A113 { namespace OSp {


template< typename _T >
struct HVEC : public std::shared_ptr< _T > {
    using std::shared_ptr< _T >::shared_ptr;
    using std::shared_ptr< _T >::operator=;

    HVEC( const std::shared_ptr< _T >&  ptr_ ) : std::shared_ptr< _T >{ ptr_ } {}
    HVEC( std::shared_ptr< _T >&& ptr_ ) : std::shared_ptr< _T >{ std::move( ptr_ ) } {}
    HVEC( _T* ptr_ ) { this->reset( ptr_ ); }
};


inline struct _LOG_BUNDLE {
    std::shared_ptr< spdlog::logger >   Default   = nullptr;
    std::shared_ptr< spdlog::logger >   IO        = nullptr;

    A113_inline spdlog::logger* operator -> () { return Default.operator->(); }

    void make_default( std::shared_ptr< spdlog::logger >& logger_ ) {
        if( nullptr != logger_ ) { logger_->set_level( spdlog::level::debug ); }
    }

} Log;

namespace st_att {
class has_logger_t {
public:
    has_logger_t() = default;
    has_logger_t( const char* logger_name_ ) : _Log{ spdlog::stdout_color_mt( logger_name_ ) } {}
    has_logger_t( HVEC< spdlog::logger > other_ ) : _Log{ std::move( other_ ) } {}

public:
    HVEC< spdlog::logger >   _Log   = nullptr;
};
};


enum InitFlags_ {
    InitFlags_None = 0x0,
    InitFlags_Sockets
};
struct init_args_t {
    int   flags   = InitFlags_None;
};
class INTERNAL {
_A113_PROTECTED:
    struct {
    #ifdef A113_TARGET_OS_WINDOWS
        WSADATA   wsa_data;
    #endif

    } _Data;

public:
    RESULT init( int argc_, char* argv_[], const init_args_t& args_ );

}; inline INTERNAL _Internal;

A113_inline RESULT init( int argc_, char* argv_[], const init_args_t& args_ ) {
    return _Internal.init( argc_, argv_, args_ );
}


struct ON_FNC_EXIT {
    typedef   std::function< void() >   proc_t;
    ON_FNC_EXIT( proc_t proc_ ) : _proc{ ( proc_t&& )proc_ } {}
    std::function< void() >   _proc;
    ~ON_FNC_EXIT() { if( nullptr != _proc ) _proc(); } 
    A113_inline void drop( void ) { _proc = nullptr; }
};


} };



