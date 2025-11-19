#pragma once
/**
 * @file: OSp/hyper_net.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/core.hpp>


namespace a113 { namespace hyn {


typedef   float   dt_t;

template< typename _T > using qlist_t = std::list< _T >;

struct exec_proc_args_t {
    dt_t   dt;
    int    sexecc;
};

class Sink;
class Drain;
class Token;


class Sink { friend class Executor;
public:
    struct config_t {
        std::string   str_id   = {};
    } config;

public:
    Sink( const config_t& config_ )
    : config{ config_ }
    {}

_A113_PROTECTED:
    qlist_t< Drain* >   _drains   = {};

public:
    virtual status_t HyN_exec_proc( Token* tok_, exec_proc_args_t args_ ) {
        return 0x0;
    }

};

class Drain { friend class Executor;
public:
    struct config_t {
        std::string   str_id     = {};

        int           _engaged   = 0x1;   
        int           _forced    = 0x0;
    } config;

public:
    Drain() = default;

    Drain( const config_t& config_ )
    : config{ config_ }
    {
        std::atomic_ref< int >{ config._engaged }.store( 0x1, std::memory_order_release );
    }

_A113_PROTECTED:
    qlist_t< Sink* >   _sinks   = {};

public:
    A113_inline void HyN_force( void ) { 
        std::atomic_ref< int >{ config._forced }.store( 0x1, std::memory_order_release ); 
    }

public:
    virtual status_t HyN_assert_proc( Token* tok_ ) = 0;

};

class Token { friend class Executor;
public:
    struct config_t {
        std::string   str_id    = {};

        int           _sexecc   = 0x0;
    } config; 

public:
    Token() = default;

_A113_PROTECTED:
    Sink*   _sink     = nullptr;

public:
    virtual status_t HyN_when_drained( Drain* drn_, status_t drn_res_ ) {
        return 0x0;
    }

    virtual HVec< Token > HyN_split( Sink* snk_ ) {
        return nullptr;
    }

    virtual status_t HyN_exec_proc( Sink* snk_, exec_proc_args_t args_ ) {
        return 0x0;
    }

};

class Executor : public st_att::_Log {
public:
    typedef   std::string_view   str_id_t;

public:
    Executor( const char* name_ ) 
    : _Log{ std::format( "{}//HyperNet_Executor//{}", A113_VERSION_STRING, name_ ) }
    {}

_A113_PROTECTED:
    qlist_t< HVec< Token > >              _tokens          = {};
    qlist_t< HVec< Sink > >               _sinks           = {};
    qlist_t< HVec< Drain > >              _drains          = {};

    std::shared_mutex                     _clock_mtx       = {};
 
    std::map< str_id_t, HVec< Sink > >    _str_id2sinks    = {};
    std::map< str_id_t, HVec< Drain > >   _str_id2drains   = {};

public:
    HVec< Sink > push_sink( HVec< Sink > snk_ );
    Sink* pull_sink_weak( str_id_t snk_ );
    A113_inline Sink* operator () ( str_id_t snk_ ) { return this->pull_sink_weak( snk_ ); }

    HVec< Drain > push_drain( HVec< Drain > drn_ );
    Drain* pull_drain_weak( str_id_t drn_ );
    A113_inline Drain* operator [] ( str_id_t drn_ ) { return this->pull_drain_weak( drn_ ); }

public:
    status_t bind_SDS( str_id_t snk1_, str_id_t drn_, str_id_t snk2_ );
    status_t bind_SD( str_id_t snk_, str_id_t drn_ );

    status_t inject( str_id_t snk_, HVec< Token > tok_ );

public:
    int clock( dt_t dt_ );

};


class Drain_Lambda : public Drain {
public:
    typedef   std::function< status_t( Token* ) >   assert_proc_t;

public:
    Drain_Lambda( const Drain::config_t& config_, assert_proc_t assert_proc_ )
    : Drain{ config_ }, _assert_proc( assert_proc_ )
    {}

_A113_PROTECTED:
    assert_proc_t   _assert_proc   = {};

public:
    virtual status_t HyN_assert_proc( Token* tok_ ) override {
        return _assert_proc( tok_ );
    }

};


} };



