#pragma once
/**
 * @file: OSp/core.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/descriptor.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <atomic>
#include <condition_variable>
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
    #define WINVER 0x0A00
    #include <winsock2.h>
    #include <ws2spi.h>
    #include <windows.h>
    #include <wincodec.h>
    #include <Ws2bth.h>
    #include <BluetoothAPIs.h>
    #include <setupapi.h>
    #include <cfgmgr32.h>
    #include <devguid.h>
    #include <initguid.h>
#endif

namespace a113 {


template< typename _T_ >
struct HVec : public std::shared_ptr< _T_ > {
    using std::shared_ptr< _T_ >::shared_ptr;
    using std::shared_ptr< _T_ >::operator=;

    HVec( const std::shared_ptr< _T_ >&  ptr_ ) : std::shared_ptr< _T_ >{ ptr_ } {}
    HVec( std::shared_ptr< _T_ >&& ptr_ ) : std::shared_ptr< _T_ >{ std::move( ptr_ ) } {}
    HVec( _T_* ptr_ ) { this->reset( ptr_ ); }
    HVec( _T_& ref_ ) : std::shared_ptr< _T_ >{ &ref_, [] ( _T_* ) static -> void {} } {}

    template< typename ...Args_ >
    A113_inline static HVec< _T_ > make( Args_&&... args_ ) { return std::make_shared< _T_ >( std::forward< Args_ >( args_ )... ); }
};


namespace st_att {
#define _A113_ST_ATT_LOG_PATTERN "[%^%l%$]\t[%c] [%n] - %v"
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

    _Log( const _Log& other_ ) = default;
    _Log( _Log&& other_ ) = default;
    _Log& operator = ( const _Log& other_ ) { _device = other_._device; return *this; }
    _Log& operator = ( _Log&& other_ ) { _device = std::move( other_._device ); return *this; }

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

public:
    A113_inline void set_log_level( spdlog::level::level_enum level_ ) { _device->set_level( level_ ); }
    A113_inline const std::string& get_name( void ) { return _device->name(); }

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
class _INTERNAL : public st_att::_Log {
_A113_PROTECTED:
    struct {
    #ifdef A113_TARGET_OS_WINDOWS
        WSADATA   wsa_data;
    #endif

    } _Data;

public:
    status_t init( int argc_, char* argv_[], const init_args_t& args_ );

public:
    using st_att::_Log::debug;
    using st_att::_Log::info;
    using st_att::_Log::warn;
    using st_att::_Log::error;
    using st_att::_Log::critical;

}; inline _INTERNAL _Internal;

A113_inline status_t init( int argc_, char* argv_[], const init_args_t& args_ ) {
    return _Internal.init( argc_, argv_, args_ );
}


struct _LOG : public st_att::_Log {
public:
    using st_att::_Log::operator=;

public:
    using st_att::_Log::debug;
    using st_att::_Log::info;
    using st_att::_Log::warn;
    using st_att::_Log::error;
    using st_att::_Log::critical;
}; inline _LOG Log{};


struct on_scope_exit_c_t {
    typedef   void(*proc_t)(void*);
    on_scope_exit_c_t( proc_t proc_, void* arg_ = NULL ) : _proc{ ( proc_t&& )proc_ }, _arg{ arg_ } {}
    proc_t   _proc;
    void*    _arg;
    ~on_scope_exit_c_t() { if( nullptr != _proc ) _proc( _arg ); } 
    A113_inline void drop( void ) { _proc = nullptr; }
};
#define A113_ON_SCOPE_EXIT_C( proc, arg ) on_scope_exit_c_t _on_scope_exit_{ proc, arg };

struct on_scope_exit_l_t {
    typedef   std::function< void() >   proc_t;
    on_scope_exit_l_t( proc_t proc_ ) : _proc{ ( proc_t&& )proc_ } {}
    proc_t   _proc;
    ~on_scope_exit_l_t() { if( nullptr != _proc ) _proc(); } 
    A113_inline void drop( void ) { _proc = nullptr; }
};
#define A113_ON_SCOPE_EXIT_L( proc ) on_scope_exit_l_t _on_scope_exit_{ proc };

#define A113_ON_SCOPE_EXIT_DROP _on_scope_exit_.drop()

#define A113_UNREACHABLE std::unreachable()

};



