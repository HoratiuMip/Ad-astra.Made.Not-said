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


#define A113_LOGI(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_General ]->info( __VA_ARGS__ ))
#define A113_LOGW(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_General ]->warn( __VA_ARGS__ ))
#define A113_LOGE(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_General ]->error( __VA_ARGS__ ))
#define A113_LOGC(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_General ]->critical( __VA_ARGS__ ))
#define A113_LOGD(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_General ]->debug( __VA_ARGS__ ))

#define A113_LOGI_IO(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IO ]->info( __VA_ARGS__ ))
#define A113_LOGW_IO(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IO ]->warn( __VA_ARGS__ ))
#define A113_LOGE_IO(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IO ]->error( __VA_ARGS__ ))
#define A113_LOGC_IO(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IO ]->critical( __VA_ARGS__ ))
#define A113_LOGD_IO(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IO ]->debug( __VA_ARGS__ ))

#define A113_LOGI_IMM(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IMM ]->info( __VA_ARGS__ ))
#define A113_LOGW_IMM(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IMM ]->warn( __VA_ARGS__ ))
#define A113_LOGE_IMM(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IMM ]->error( __VA_ARGS__ ))
#define A113_LOGC_IMM(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IMM ]->critical( __VA_ARGS__ ))
#define A113_LOGD_IMM(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_IMM ]->debug( __VA_ARGS__ ))

#define A113_LOGI_SCT(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_SCT ]->info( __VA_ARGS__ ))
#define A113_LOGW_SCT(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_SCT ]->warn( __VA_ARGS__ ))
#define A113_LOGE_SCT(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_SCT ]->error( __VA_ARGS__ ))
#define A113_LOGC_SCT(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_SCT ]->critical( __VA_ARGS__ ))
#define A113_LOGD_SCT(...) (a113::_Internal._Component_loggers[ a113::_INTERNAL::LogComponent_SCT ]->debug( __VA_ARGS__ ))



enum InitFlags_ {
    InitFlags_None = 0x0,
    InitFlags_Sockets
};
struct init_args_t {
    int   flags   = InitFlags_None;
};
class _INTERNAL {
public:
    _INTERNAL( void ) {
    #define _MAKE_LOG_AND_PATERN( c, s ) _Component_loggers[ c ] = spdlog::stdout_color_mt( A113_VERSION_STRING s ); _Component_loggers[ c ]->set_pattern( "[%^%l%$] [%c] [%n] - %v" );
        _MAKE_LOG_AND_PATERN( LogComponent_General, "--General" )     
        _MAKE_LOG_AND_PATERN( LogComponent_IO, "--Input/Output" );
        _MAKE_LOG_AND_PATERN( LogComponent_IMM, "--Immersion" );
        _MAKE_LOG_AND_PATERN( LogComponent_SCT, "--Structure" );
    #undef _MAKE_LOG_AND_PATERN
    }

_A113_PROTECTED:
    struct {
    #ifdef A113_TARGET_OS_WINDOWS
        WSADATA   wsa_data;
    #endif

    } _Data;

public:
    status_t init( int argc_, char* argv_[], const init_args_t& args_ );

public:
    enum LogComponent_ {
        LogComponent_General, LogComponent_IO, LogComponent_IMM, LogComponent_SCT, _LogComponent_COUNT
    };
    std::shared_ptr< spdlog::logger >   _Component_loggers[ _LogComponent_COUNT ] = { nullptr };

}; inline _INTERNAL _Internal;

A113_inline status_t init( int argc_, char* argv_[], const init_args_t& args_ ) {
    return _Internal.init( argc_, argv_, args_ );
}


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



