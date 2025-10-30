/**
 * @file: OSp/hyper_net.cpp
 * @brief: Implementation file.
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/hyper_net.hpp>


namespace A113 { namespace OSp {


A113_OS_FNC int HyperNet_Executor::clock( hyper_net_dt_t dt_ ) {
    RESULT     result      = 0x0;
    auto       tok         = _tokens.begin();
    const int  tok_cnt_lim = _tokens.size();
    int        tok_cnt     = 0x0;

    std::unique_lock clock_lock{ _clock_mtx };
    for(; tok != _tokens.end() && tok_cnt < tok_cnt_lim; ++tok_cnt ) {
        for( auto& drn : (*tok)->_sink->_drains ) {
            if( 0x0 == std::atomic_ref< int >{ drn->config._engaged }.load( std::memory_order_relaxed ) ) continue;
            
            result = drn->HyN_assert_proc( &**tok );
            if( 0x0 == result ) goto l_exec_token;
            _Log->debug( "DRAIN[\"{}\"] asserted TOKEN[\"{}\"] from SINK[\"{}\"].", drn->config.str_id, (*tok)->config.str_id,(*tok)->_sink->config.str_id );

            result = (*tok)->HyN_when_drained( drn, result );
            if( 0x0 != result || drn->_sinks.empty() ) goto l_pop_token;

            auto drn_snk = drn->_sinks.begin();
            (*tok)->_sink = *drn_snk;
            (*tok)->config._sexecc = 0x0;

            ++drn_snk;
            for(; drn_snk != drn->_sinks.end(); ++drn_snk ) {
                auto spl_tok = (*tok)->HyN_split( *drn_snk );
                if( nullptr != spl_tok ) _tokens.emplace_back( std::move( spl_tok ) );
            }
        }
        goto l_end;

    l_exec_token: {
        hyper_net_exec_proc_args_t proc_args = {
            dt:     dt_,
            sexecc: (*tok)->config._sexecc 
        };

        result = (*tok)->_sink->HyN_exec_proc( &**tok, proc_args );
        if( 0x0 != result ) goto l_pop_token;

        result = (*tok)->HyN_exec_proc( (*tok)->_sink, proc_args );
        if( 0x0 != result ) goto l_pop_token;

        ++(*tok)->config._sexecc;
        goto l_end;
    }
    l_pop_token:
        _Log->debug( "Removed TOKEN[\"{}\"] due to RESULT[{}].", (*tok)->config.str_id, result );
        tok = _tokens.erase( tok );
        continue;

    l_end:
        ++tok;
    }

    return tok_cnt;
}


A113_OS_FNC HVEC< HyperNet_Sink > HyperNet_Executor::push_sink( HVEC< HyperNet_Sink > snk_ ) {
    HVEC< HyperNet_Sink >& snk = _sinks.emplace_back( std::move( snk_ ) );
    _str_id2sinks[ snk->config.str_id ] = snk;
    return snk;
}

A113_OS_FNC HyperNet_Sink* HyperNet_Executor::pull_sink_weak( str_id_t snk_ ) {
    auto itr = _str_id2sinks.find( snk_ );
    return itr != _str_id2sinks.end() ? &*(itr->second) : nullptr;
}


A113_OS_FNC HVEC< HyperNet_Drain > HyperNet_Executor::push_drain( HVEC< HyperNet_Drain > drn_ ) {
    auto& drn = _drains.emplace_back( std::move( drn_ ) );
    _str_id2drains[ drn->config.str_id ] = drn;
    return drn;
}

A113_OS_FNC HyperNet_Drain* HyperNet_Executor::pull_drain_weak( str_id_t drn_str_id_ ) {
    auto itr = _str_id2drains.find( drn_str_id_ );
    return itr != _str_id2drains.end() ? &*(itr->second) : nullptr;
}


A113_OS_FNC RESULT HyperNet_Executor::bind_SDS( str_id_t snk1_, str_id_t drn_, str_id_t snk2_ ) {
    auto* drn = this->pull_drain_weak( drn_ );
    this->pull_sink_weak( snk1_ )->_drains.push_back( drn );
    drn->_sinks.push_back( this->pull_sink_weak( snk2_ ) );
    return 0x0;
}

A113_OS_FNC RESULT HyperNet_Executor::bind_SD( str_id_t snk_, str_id_t drn_ ) {
    this->pull_sink_weak( snk_ )->_drains.push_back( this->pull_drain_weak( drn_ ) );
    return 0x0;
}

A113_OS_FNC RESULT HyperNet_Executor::inject( str_id_t snk_, HVEC< HyperNet_Token > tok_ ) {
    std::unique_lock clock_lock{ _clock_mtx };

    auto* snk = this->pull_sink_weak( snk_ );
    auto& tok = _tokens.emplace_back( std::move( tok_ ) );
    tok->_sink = snk;

    _Log->debug( "TOKEN[\"{}\"] injected in SINK[\"{}\"].", tok->config.str_id, snk->config.str_id );

    return 0x0;
}


} };



