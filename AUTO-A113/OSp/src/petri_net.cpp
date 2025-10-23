#pragma once
/**
 * @file: OSp/petri_net.cpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>


namespace A113 { namespace OSp {


A113_OS_FNC PetriNet_Executor::impulse( dt_t dt_ ) {
    RESULT     result      = 0x0;
    auto       tok         = _tokens.begin();
    const int  tok_cnt_lim = _tokens.size();
    int        tok_cnt     = 0x0;

    for(; tok != _tokens.end() && tok_cnt < tok_cnt_lim;) {
        for( auto& drn : tok->_sink->_drains ) {
            if( 0x0 == drn->config.engaged ) continue;
            
            result = drn->assert_token( &*tok );
            if( 0x0 == result ) continue;

            result = tok->when_drained( drn, result );
            if( 0x0 != result ) goto l_pop_token;

            auto drn_snk = drn->_sinks.begin();
            tok->_sink = &*drn_snk;

            ++drn_snk;
            for(; drn_snk != drn->_sinks.end(); ++drn_snk ) {
                auto spl_tok = tok->split( &*drn_snk );
                if( nullptr != spl_tok ) _tokens.emplace_back( std::move( spl_tok ) );
            }
        }
    
    l_pop_token:
        tok = _tokens.erase( tok );

        ++tok_cnt;
    }
}


} };



