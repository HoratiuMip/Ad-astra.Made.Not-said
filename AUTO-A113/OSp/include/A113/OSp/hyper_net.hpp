#pragma once
/**
 * @file: OSp/hyper_net.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/core.hpp>


namespace A113 { namespace OSp {


typedef   float   hyper_net_dt_t;

struct hyper_net_exec_proc_args_t {
    hyper_net_dt_t   dt;
    int              sexecc;
};

class HyperNet_Sink;
class HyperNet_Drain;
class HyperNet_Token;


class HyperNet_Sink { friend class HyperNet_Executor;
public:
    struct config_t {
        std::string   str_id   = {};
    } config;

public:
    HyperNet_Sink( const config_t& config_ )
    : config{ config_ }
    {}

_A113_PROTECTED:
    std::list< HyperNet_Drain* >   _drains   = {};

public:
    virtual RESULT HyN_exec_proc( HyperNet_Token* tok_, hyper_net_exec_proc_args_t args_ ) {
        return 0x0;
    }

};

class HyperNet_Drain { friend class HyperNet_Executor;
public:
    struct config_t {
        std::string   str_id     = {};

        int           _engaged   = 0x1;   
        int           _forced    = 0x0;
    } config;

public:
    HyperNet_Drain() = default;

    HyperNet_Drain( const config_t& config_ )
    : config{ config_ }
    {
        std::atomic_ref< int >{ config._engaged }.store( 0x1, std::memory_order_release );
    }

_A113_PROTECTED:
    std::list< HyperNet_Sink* >   _sinks   = {};

public:
    A113_inline void HyN_force( void ) { 
        std::atomic_ref< int >{ config._forced }.store( 0x1, std::memory_order_release ); 
    }

public:
    virtual RESULT HyN_assert_proc( HyperNet_Token* tok_ ) = 0;

};

class HyperNet_Token { friend class HyperNet_Executor;
public:
    struct config_t {
        std::string   str_id    = {};

        int           _sexecc   = 0x0;
    } config; 

public:
    HyperNet_Token() = default;

_A113_PROTECTED:
    HyperNet_Sink*   _sink     = nullptr;

public:
    virtual RESULT HyN_when_drained( HyperNet_Drain* drn_, RESULT drn_res_ ) {
        return 0x0;
    }

    virtual HVEC< HyperNet_Token > HyN_split( HyperNet_Sink* snk_ ) {
        return nullptr;
    }

    virtual RESULT HyN_exec_proc( HyperNet_Sink* snk_, hyper_net_exec_proc_args_t args_ ) {
        return 0x0;
    }

};

class HyperNet_Executor {
public:
    typedef   std::string_view   str_id_t;

public:
    HyperNet_Executor( const char* name_, bool show_logs_ = false ) {
        _Log = spdlog::stdout_color_mt( std::format( "{}//HyperNet_Executor//{}", A113_VERSION_STRING, name_ ) );
        _Log->set_level( show_logs_ ? spdlog::level::debug : spdlog::level::info );
    }

public:
     std::shared_ptr< spdlog::logger >             _Log             = nullptr;

_A113_PROTECTED:
    std::list< HVEC< HyperNet_Token > >            _tokens          = {};
    std::list< HVEC< HyperNet_Sink > >             _sinks           = {};
    std::list< HVEC< HyperNet_Drain > >            _drains          = {};

    std::shared_mutex                              _clock_mtx       = {};
 
    std::map< str_id_t, HVEC< HyperNet_Sink > >    _str_id2sinks    = {};
    std::map< str_id_t, HVEC< HyperNet_Drain > >   _str_id2drains   = {};

public:
    HVEC< HyperNet_Sink > push_sink( HVEC< HyperNet_Sink > snk_ );
    HyperNet_Sink* pull_sink_weak( str_id_t snk_ );
    A113_inline HyperNet_Sink* operator () ( str_id_t snk_ ) { return this->pull_sink_weak( snk_ ); }

    HVEC< HyperNet_Drain > push_drain( HVEC< HyperNet_Drain > drn_ );
    HyperNet_Drain* pull_drain_weak( str_id_t drn_ );
    A113_inline HyperNet_Drain* operator [] ( str_id_t drn_ ) { return this->pull_drain_weak( drn_ ); }

public:
    RESULT bind_SDS( str_id_t snk1_, str_id_t drn_, str_id_t snk2_ );
    RESULT bind_SD( str_id_t snk_, str_id_t drn_ );

    RESULT inject( str_id_t snk_, HVEC< HyperNet_Token > tok_ );

public:
    int clock( hyper_net_dt_t dt_ );

};


class HyperNet_Drain_Lambda : public HyperNet_Drain {
public:
    typedef   std::function< RESULT( HyperNet_Token* ) >   assert_proc_t;

public:
    HyperNet_Drain_Lambda( const HyperNet_Drain::config_t& config_, assert_proc_t assert_proc_ )
    : HyperNet_Drain{ config_ }, _assert_proc( assert_proc_ )
    {}

_A113_PROTECTED:
    assert_proc_t   _assert_proc   = {};

public:
    virtual RESULT HyN_assert_proc( HyperNet_Token* tok_ ) override {
        return _assert_proc( tok_ );
    }

};


} };



