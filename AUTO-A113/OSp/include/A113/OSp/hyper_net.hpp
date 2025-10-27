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
        std::string   name   = {};
    } config;

public:
    HyperNet_Sink( const config_t& config_ )
    : config{ config_ }
    {}

A113_PROTECTED:
    std::list< HyperNet_Drain* >   _drains   = {};

public:
    virtual RESULT exec_proc( HyperNet_Token* tok_, hyper_net_exec_proc_args_t args_ ) {
        return 0x0;
    }

};

class HyperNet_Drain { friend class HyperNet_Executor;
public:
    struct config_t {
        std::string   name       = {};
        int           _engaged   = 0x1;   
    } config;

public:
    HyperNet_Drain() = default;

    HyperNet_Drain( const config_t& config_ )
    : config{ config_ }
    {
        std::atomic_ref< int >{ config._engaged }.store( 0x1, std::memory_order_release );
    }

A113_PROTECTED:
    std::list< HyperNet_Sink* >   _sinks   = {};

public:
    virtual RESULT assert_proc( HyperNet_Token* tok_ ) = 0;

};

class HyperNet_Token { friend class HyperNet_Executor;
public:
    struct config_t {
        std::string   name      = {};
        int           _sexecc   = 0x0;
    } config; 

public:
    HyperNet_Token() = default;

A113_PROTECTED:
    HyperNet_Sink*   _sink     = nullptr;

public:
    virtual RESULT when_drained( HyperNet_Drain* drn_, RESULT drn_res_ ) {
        return 0x0;
    }

    virtual HPtr< HyperNet_Token > split( HyperNet_Sink* snk_ ) {
        return nullptr;
    }

    virtual RESULT exec_proc( HyperNet_Sink* snk_, hyper_net_exec_proc_args_t args_ ) {
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
     std::shared_ptr< spdlog::logger >             _Log       = nullptr;

A113_PROTECTED:
    std::list< HPtr< HyperNet_Token > >            _tokens   = {};
    std::list< HPtr< HyperNet_Sink > >             _sinks    = {};
    std::list< HPtr< HyperNet_Drain > >            _drains   = {};

    std::map< str_id_t, HPtr< HyperNet_Sink > >    _name2sinks   = {};
    std::map< str_id_t, HPtr< HyperNet_Drain > >   _name2drains   = {};

public:
    HPtr< HyperNet_Sink > push_sink( HPtr< HyperNet_Sink > snk_ );
    HyperNet_Sink* pull_sink_weak( str_id_t snk_name_ );
    A113_inline HyperNet_Sink* operator () ( str_id_t snk_name_ ) { return this->pull_sink_weak( snk_name_ ); }

    HPtr< HyperNet_Drain > push_drain( HPtr< HyperNet_Drain > drn_ );
    HyperNet_Drain* pull_drain_weak( str_id_t drn_name_ );
    A113_inline HyperNet_Drain* operator [] ( str_id_t drn_name_ ) { return this->pull_drain_weak( drn_name_ ); }

public:
    RESULT bind_SDS( str_id_t snk1_, str_id_t drn_, str_id_t snk2_ );
    RESULT bind_SD( str_id_t snk_, str_id_t drn_ );

    RESULT insert( str_id_t snk_, HPtr< HyperNet_Token > tok_ );

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

A113_PROTECTED:
    assert_proc_t   _assert_proc   = {};

public:
    virtual RESULT assert_proc( HyperNet_Token* tok_ ) override {
        return _assert_proc( tok_ );
    }

};


} };



