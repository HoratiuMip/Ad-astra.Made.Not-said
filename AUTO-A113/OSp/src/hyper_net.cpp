/**
 * @file: OSp/hyper_net.cpp
 * @brief: Implementation file.
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/hyper_net.hpp>


namespace a113 { namespace hyn {


A113_IMPL_FNC int Executor::clock( dt_t dt_ ) {
    status_t   status      = 0x0;
    auto       tok_itr     = _tokens.begin();
    const int  tok_cnt_lim = _tokens.size();
    int        tok_cnt     = 0x0;

    std::lock_guard clock_lock{ _clock_mtx };
    for(; tok_itr != _tokens.end() && tok_cnt < tok_cnt_lim; ++tok_cnt ) {
        bool tok_itr_modified = false;

        if( (*tok_itr)->_runtime.last_rte_clk == _clock ) goto l_end;

        for( Route* rte : (*tok_itr)->_runtime.prt->_routes ) {
            if( rte->_runtime.last_rte_clk == _clock ) continue;

            A113_ON_SCOPE_EXIT_L( ([ rte, this ] ( void ) -> void { rte->_runtime.last_rte_clk = _clock; }) );

            {
            bool toks_on_all_inputs = true;
            for( Route::in_plan_t& in : rte->_inputs ) if( in.min_tok_cnt > in.prt->_runtime.toks.size() ) { toks_on_all_inputs = false; break; }
            if( not toks_on_all_inputs ) continue;
            }

            qlist_t< Route::in_plan_t >::iterator  in_itr  = rte->_inputs.begin();
            qlist_t< Route::out_plan_t >::iterator out_itr = rte->_outputs.begin(), eff_out_itr = out_itr;
            
            for(; in_itr != rte->_inputs.end(); ++in_itr ) {
                qlist_t< qlist_t< HVec< Token > >::iterator >::iterator in_prt_tok_itr = in_itr->prt->_runtime.toks.begin();

                for( int n = 1; n <= in_itr->rte_tok_cnt && in_prt_tok_itr != in_itr->prt->_runtime.toks.end(); ++n ) {
                    qlist_t< HVec< Token > >::iterator flight_tok_itr = *in_prt_tok_itr;

                    eff_out_itr = out_itr;

                    in_prt_tok_itr = in_itr->prt->_runtime.toks.erase( in_prt_tok_itr );
                    if( eff_out_itr == rte->_outputs.end() || -0x1 == in_itr->flight_mode ) {
                        A113_LOGD_SCT( "ROUTE[\"{}\"] ejected TOKEN[\"{}\"].", rte->config.str_id, (*flight_tok_itr)->config.str_id );

                        if( tok_itr == flight_tok_itr ) { tok_itr_modified = true; tok_itr = _tokens.erase( tok_itr ); }
                        else _tokens.erase( flight_tok_itr );
                        continue;
                    }
                    
                    eff_out_itr->prt->_runtime.toks.push_back( flight_tok_itr );
                    (*flight_tok_itr)->_runtime.prt          = eff_out_itr->prt;
                    (*flight_tok_itr)->_runtime.last_rte_clk = _clock;
                    rte->_runtime.last_rte_clk               = _clock;

                    A113_LOGD_SCT( "ROUTE[\"{}\"] flew TOKEN[\"{}\"] from PORT[\"{}\"] to PORT[\"{}\"].", rte->config.str_id, (*flight_tok_itr)->config.str_id, in_itr->prt->config.str_id, eff_out_itr->prt->config.str_id );

                    ++eff_out_itr;
                    for( int m = 1; m <= in_itr->flight_mode && eff_out_itr != rte->_outputs.end(); ++m, ++eff_out_itr ) {
                        Token* spl_tok = &**eff_out_itr->prt->_runtime.toks.emplace_back( _tokens.insert( _tokens.end(), (*flight_tok_itr)->HyN_split( m, *rte ) ) );
                        spl_tok->_runtime.prt = eff_out_itr->prt;
                        A113_LOGD_SCT( "TOKEN[\"{}\"] splitted in PORT[\"{}\"].", spl_tok->config.str_id, eff_out_itr->prt->config.str_id );
                    }
                }

                out_itr = eff_out_itr;
            }
            
            if( out_itr != rte->_outputs.end() ) A113_LOGW_SCT( "ROUTE[\"{}\"] did not fill all output ports.", rte->config.str_id );
        }

    l_end:
        if( not tok_itr_modified ) ++tok_itr;
    }

    ++_clock;
    return tok_cnt;
}


A113_IMPL_FNC HVec< Port > Executor::push_port( HVec< Port > prt_ ) {
    HVec< Port >& prt = _ports.emplace_back( std::move( prt_ ) );
    _str_id2ports[ prt->config.str_id ] = prt;

    A113_LOGD_SCT( "Pushed PORT[\"{}\"].", prt->config.str_id );
    return prt;
}

A113_IMPL_FNC Port* Executor::pull_port_weak( str_id_t prt_ ) {
    auto itr = _str_id2ports.find( prt_ );
    return itr != _str_id2ports.end() ? &*(itr->second) : nullptr;
}


A113_IMPL_FNC HVec< Route > Executor::push_route( HVec< Route > rte_ ) {
    auto& rte = _routes.emplace_back( std::move( rte_ ) );
    _str_id2routes[ rte->config.str_id ] = rte;

    A113_LOGD_SCT( "Pushed ROUTE[\"{}\"].", rte->config.str_id );
    return rte;
}

A113_IMPL_FNC Route* Executor::pull_route_weak( str_id_t rte_str_id_ ) {
    auto itr = _str_id2routes.find( rte_str_id_ );
    return itr != _str_id2routes.end() ? &*(itr->second) : nullptr;
}


A113_IMPL_FNC status_t Executor::bind_PRP( str_id_t in_prt_, str_id_t rte_, str_id_t out_prt_, const Route::in_plan_t& in_pln_, const Route::out_plan_t& out_pln_ ) {
    Route* rte     = this->pull_route_weak( rte_ );    A113_ASSERT_OR( rte ) { A113_LOGE_SCT( "ROUTE[\"{}\"] does not exist.", rte_ ); return -0x1; }
    Port*  in_prt  = this->pull_port_weak( in_prt_ );  A113_ASSERT_OR( in_prt ) { A113_LOGE_SCT( "Input PORT[\"{}\"] does not exist.", in_prt_ ); return -0x1; }
    Port*  out_prt = this->pull_port_weak( out_prt_ ); A113_ASSERT_OR( out_prt ) { A113_LOGE_SCT( "Output PORT[\"{}\"] does not exist.", out_prt_ ); return -0x1; }

    in_prt->_routes.push_back( rte );
    rte->_inputs.emplace_back( in_pln_ ).prt   = in_prt;
    rte->_outputs.emplace_back( out_pln_ ).prt = out_prt;

    A113_LOGD_SCT( "Bound PORT[\"{}\"] -> ROUTE[\"{}\"] -> PORT[\"{}\"].", in_prt->config.str_id, rte->config.str_id, out_prt->config.str_id );
    return 0x0;
}

A113_IMPL_FNC status_t Executor::bind_RP( str_id_t rte_, str_id_t out_prt_, const Route::out_plan_t& out_pln_ ) {
    Route* rte     = this->pull_route_weak( rte_ );    A113_ASSERT_OR( rte ) { A113_LOGE_SCT( "ROUTE[\"{}\"] does not exist.", rte_ ); return -0x1; }
    Port*  out_prt = this->pull_port_weak( out_prt_ ); A113_ASSERT_OR( out_prt ) { A113_LOGE_SCT( "Output PORT[\"{}\"] does not exist.", out_prt_ ); return -0x1; }

    rte->_outputs.emplace_back( out_pln_ ).prt = out_prt;

    A113_LOGD_SCT( "Bound ROUTE[\"{}\"] -> PORT[\"{}\"].", rte->config.str_id, out_prt->config.str_id );
    return 0x0;
}

A113_IMPL_FNC status_t Executor::bind_PR( str_id_t in_prt_, str_id_t rte_, const Route::in_plan_t& in_pln_ ) {
    Port*  in_prt = this->pull_port_weak( in_prt_ ); A113_ASSERT_OR( in_prt ) { A113_LOGE_SCT( "Input PORT[\"{}\"] does not exist.", in_prt_ ); return -0x1; }
    Route* rte    = this->pull_route_weak( rte_ );   A113_ASSERT_OR( rte ) { A113_LOGE_SCT( "ROUTE[\"{}\"] does not exist.", rte_ ); return -0x1; }

    in_prt->_routes.push_back( rte );
    rte->_inputs.emplace_back( in_pln_ ).prt = in_prt;

    A113_LOGD_SCT( "Bound PORT[\"{}\"] -> ROUTE[\"{}\"].", in_prt->config.str_id, rte->config.str_id );
    return 0x0;
}

A113_IMPL_FNC status_t Executor::inject( str_id_t prt_, HVec< Token > tok_ ) {
    std::unique_lock clock_lock{ _clock_mtx };

    Port* prt     = this->pull_port_weak( prt_ );
    auto  tok_itr = _tokens.insert( _tokens.end(), std::move( tok_ ) );

    (**tok_itr)._runtime.req_assert = A113_STRUCT_HAS_OVR( (**tok_itr), Token::HyN_assert );    

    ( *prt->_runtime.toks.emplace_back( tok_itr ) )->_runtime.prt = prt;

    A113_LOGD_SCT( "TOKEN[\"{}\"] injected in PORT[\"{}\"].", (*tok_itr)->config.str_id, prt->config.str_id );

    return 0x0;
}


} };



