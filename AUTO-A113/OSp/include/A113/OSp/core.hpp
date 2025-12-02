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

namespace a113 {


template< typename _T >
struct HVec : public std::shared_ptr< _T > {
    using std::shared_ptr< _T >::shared_ptr;
    using std::shared_ptr< _T >::operator=;

    HVec( const std::shared_ptr< _T >&  ptr_ ) : std::shared_ptr< _T >{ ptr_ } {}
    HVec( std::shared_ptr< _T >&& ptr_ ) : std::shared_ptr< _T >{ std::move( ptr_ ) } {}
    HVec( _T* ptr_ ) { this->reset( ptr_ ); }
};


namespace st_att {
#define _A113_ST_ATT_LOG_PATTERN "[%c] [%^%l%$] [%n] - %v"
class _Log {
_A113_PROTECTED:
    template< typename _T >
    HVec< spdlog::logger > _make_device( _T&& name_ ) {
        return spdlog::stdout_color_mt( std::forward< _T >( name_ ) );
    }

    void _drop_device( void ) {
        if( _device ) spdlog::drop( _device->name() );
    }

public:
#define _A113_ST_ATT__SET_LOG_PATTERN _device->set_pattern( _A113_ST_ATT_LOG_PATTERN )
    _Log() = default;
    _Log( const char* name_ ) : _device{ this->_make_device( name_ ) } {
        _A113_ST_ATT__SET_LOG_PATTERN;
    }
    _Log( const std::string& name_ ) : _device{ this->_make_device( name_ ) } {
        _A113_ST_ATT__SET_LOG_PATTERN;
    }
    _Log( HVec< spdlog::logger > other_ ) : _device{ std::move( other_ ) } {}

    ~_Log() { this->_drop_device(); }

_A113_PRIVATE:
    HVec< spdlog::logger >   _device   = nullptr;

_A113_PROTECTED:
#define _A113_ST_ATT__LOG_FWD_FNC( fnc ) template< typename ...Args > A113_inline void fnc( const spdlog::format_string_t< Args... > fmt_, Args&&... args_ ) const { _device->fnc( fmt_, std::forward< Args >( args_ )... ); }
    _A113_ST_ATT__LOG_FWD_FNC( debug )
    _A113_ST_ATT__LOG_FWD_FNC( info )
    _A113_ST_ATT__LOG_FWD_FNC( warn )
    _A113_ST_ATT__LOG_FWD_FNC( error )
    _A113_ST_ATT__LOG_FWD_FNC( critical )
#undef _A113_ST_ATT__LOG_FWD_FNC

_A113_PROTECTED:
    template< typename _T >
    A113_inline void morph( _T&& name_ ) {
        this->_drop_device();
        _device = this->_make_device( std::forward< _T >( name_ ) ); _A113_ST_ATT__SET_LOG_PATTERN;
    }
#undef _A113_ST_ATT__SET_LOG_PATTERN
};
};


enum InitFlags_ {
    InitFlags_None = 0x0,
    InitFlags_Sockets
};
struct init_args_t {
    int   flags   = InitFlags_None;
};
class INTERNAL : st_att::_Log {
_A113_PROTECTED:
    struct {
    #ifdef A113_TARGET_OS_WINDOWS
        WSADATA   wsa_data;
    #endif

    } _Data;

public:
    status_t init( int argc_, char* argv_[], const init_args_t& args_ );

}; inline INTERNAL _Internal;

A113_inline status_t init( int argc_, char* argv_[], const init_args_t& args_ ) {
    return _Internal.init( argc_, argv_, args_ );
}


struct ON_FNC_EXIT {
    typedef   std::function< void() >   proc_t;
    ON_FNC_EXIT( proc_t proc_ ) : _proc{ ( proc_t&& )proc_ } {}
    std::function< void() >   _proc;
    ~ON_FNC_EXIT() { if( nullptr != _proc ) _proc(); } 
    A113_inline void drop( void ) { _proc = nullptr; }
};


};



