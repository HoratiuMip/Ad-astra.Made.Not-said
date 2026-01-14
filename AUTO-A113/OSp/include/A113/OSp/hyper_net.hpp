#pragma once
/**
 * @file: OSp/hyper_net.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>


namespace a113 { namespace hyn {


typedef   float   dt_t;

template< typename _T > using qlist_t = std::list< _T >;

struct tok_exec_args_t {
    dt_t   dt;
    int    sexecc;
};

class Port;
class Route;
class Token;


class Port { friend class Executor;
public:
    struct config_t {
        std::string   str_id   = {};
    } config;

_A113_PROTECTED:
    struct _runtime_t {
        qlist_t< qlist_t< HVec< Token > >::iterator >   toks   = {};
    } _runtime;

public:
    Port( const config_t& config_ )
    : config{ config_ }
    {}

_A113_PROTECTED:
    qlist_t< Route* >   _routes   = {};

public:
    virtual status_t HyN_tok_exec( Token* tok_, const tok_exec_args_t& args_ ) {
        return 0x0;
    }

};

class Route { friend class Executor;
public:
    struct config_t {
        std::string   str_id   = {};
    } config;

_A113_PROTECTED:
    struct _runtime_t {
        int   last_rte_clk      = 0x0;
        int   crt_clk_rte_cnt   = 0;
    } _runtime;

public:
    struct in_plan_t {
        Port*   prt           = nullptr;

        int     flight_mode   = 0x0;
        int     min_tok_cnt   = 1;
        int     rte_tok_cnt   = 1;
    };

    struct out_plan_t {
        Port*   prt   = nullptr;
    };

public:
    Route() = default;

    Route( const config_t& config_ )
    : config{ config_ }
    {}

_A113_PROTECTED:
    qlist_t< in_plan_t >    _inputs    = {};
    qlist_t< out_plan_t >   _outputs   = {};

public:
    virtual status_t HyN_assert_input( Token* tok_ ) {
        return 0x1;
    }

};

class Token { friend class Executor;
public:
    struct config_t {
        std::string   str_id      = {};
    } config; 

_A113_PROTECTED:
    struct _runtime_t {
        Port*   prt            = nullptr;
        int     last_rte_clk   = 0x0;
        int     sexecc         = 0;
        bool    req_assert     = false;
    } _runtime;

public:
    Token() = default;

    Token( const config_t& config_ )
    : config{ config_ }
    {}

_A113_PROTECTED:

public:
    virtual status_t HyN_when_routed( Route* rte_, status_t rte_status_ ) {
        return 0x0;
    }

    virtual status_t HyN_assert( Route* rte_ ) {
        return 0x1;
    }

    virtual HVec< Token > HyN_split( int offset_, Route& rte_ ) {
        return new Token{};
    }

};

class Executor {
public:
    typedef   std::string_view   str_id_t;

public:
    Executor( const char* name_ ) {}

_A113_PROTECTED:
    int                                   _clock           = 0x0;

    qlist_t< HVec< Token > >              _tokens          = {};
    qlist_t< HVec< Port > >               _ports           = {};
    qlist_t< HVec< Route > >              _routes          = {};

    std::shared_mutex                     _clock_mtx       = {};
 
    std::map< str_id_t, HVec< Port > >    _str_id2ports    = {};
    std::map< str_id_t, HVec< Route > >   _str_id2routes   = {};

public:
    HVec< Port > push_port( HVec< Port > prt_ );
    Port* pull_port_weak( str_id_t prt_ );
    A113_inline Port* operator () ( str_id_t prt_ ) { return this->pull_port_weak( prt_ ); }

    HVec< Route > push_route( HVec< Route > rte_ );
    Route* pull_route_weak( str_id_t rte_ );
    A113_inline Route* operator [] ( str_id_t rte_ ) { return this->pull_route_weak( rte_ ); }

public:
    status_t bind_PRP( str_id_t in_prt_, str_id_t rte_, str_id_t out_prt_, const Route::in_plan_t& in_pln_, const Route::out_plan_t& out_pln_ );
    status_t bind_RP( str_id_t rte_, str_id_t out_prt_, const Route::out_plan_t& out_pln_ );
    status_t bind_PR( str_id_t in_prt_, str_id_t rte_, const Route::in_plan_t& in_pln_ );

    status_t inject( str_id_t prt_, HVec< Token > tok_ );

public:
    status_t integrity_assert( void );

public:
    int clock( dt_t dt_ );

public:
    std::string Utils_make_graphviz( void ) {
        std::string acc = ""; 

        for( auto& rte : _routes ) {
            for( auto& in : rte->_inputs ) acc += in.prt->config.str_id + "->" + rte->config.str_id + '\n';
            for( auto& out : rte->_outputs ) acc += rte->config.str_id + "->" + out.prt->config.str_id + '\n';
            acc += std::format( "{0:}[shape=box, width=.1, height=1, xlabel=\"{0:}\", label=\"\"]\n", rte->config.str_id ); 
        }

        for( auto& prt : _ports ) {
            acc += std::format( 
                "{0:}[label=\"{0:}\\n({1:})\",style=filled,color={2:}]\n", 
                prt->config.str_id, 
                prt->_runtime.toks.size(),
                prt->_runtime.toks.empty() ? "lightgray" : "darkgray"
            );
        }

        return std::format(
            "digraph G {{ rankdir=\"LR\" label=\"{}\"\n" \
            "graph [fontname = \"Comic Sans MS\"]\n"     \
            "node [fontname = \"Comic Sans MS\"]\n"      \
            "edge [fontname = \"Comic Sans MS\"]\n"      \
            "{}"                                         \
            "}}"
        , "a113::HyN::Executor", acc );
    }

};


} };



