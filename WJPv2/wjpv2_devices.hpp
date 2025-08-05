#ifndef WJPV2_DEVICES_HPP
#define WJPV2_DEVICES_HPP
/*===== Warp Joint Protocol v2 - Devices - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Yes.
|
======*/
#include "wjpv2_core.hpp"
#include "wjpv2_bridges.hpp"
#include "wjpv2_structs.hpp"

#include "wjpv2_bridges_RTG.hpp"



struct WJP_WBCK_Info_RTG {
    WJP_Interlocked< bool >   resolved     = { false };
    int                       recv_count   = 0;
    WJPErr_                   err          = WJPErr_None;
};

struct WJP_RSLV_Info_RTG {
    WJP_Head   head         = {};
    int        sent_count   = 0;
    WJPErr_    err          = WJPErr_None;
};

struct _WJP_RSLV_Context_RTG {
    WJP_RSLV_Info_RTG*   info         = nullptr;
    int32_t              recv_count   = 0;
    int                  flags        = 0;
};

struct WJP_WBCK_Resolver_RTG {
#if defined( _WJP_SEMANTICS_AGGREGATES_REQUIRE_CONSTRUCTOR )
    WJP_WBCK_Resolver_RTG() = default;

    WJP_WBCK_Resolver_RTG( WJP_WBCK_Info_RTG* info_, int16_t noun_ )
    : info{ info_ }, noun{ noun_ } {}
#endif

    ~WJP_WBCK_Resolver_RTG() {
        info->resolved.write( true, WJP_MEM_ORD_RELEASE );
        info->resolved.broadcast();
    }

    WJP_WBCK_Info_RTG*    info   = nullptr;
    int16_t               noun   = 0;
};

struct WJP_DEVICE_Euclid_RTG {
    WJP_BRIDGE_InterMech*                        _inter_mech      = nullptr;

    WJP_Interlocked< int16_t >                   _wbck_nouner     = { 0 };

    WJP_CircularQueue< WJP_WBCK_Resolver_RTG >   _resolvers       = {};

    WJP_Mutex                                    _mtx_resolvers   = {};
    WJP_Mutex                                    _mtx_send        = {};

    void*                                        _arg             = nullptr;

    WJP_BRIDGE_LMHIReceiver*                     _lmhi_receiver   = nullptr;

    WJP_MDsc_v                                   _recv_buffer     = {};

    void bind_inter_mech( WJP_BRIDGE_InterMech* inter_mech ) { _inter_mech = inter_mech; }
    void bind_resolvers_mdsc( WJP_MDsc< WJP_WBCK_Resolver_RTG > mdsc ) { _resolvers._mdsc = mdsc; }
    void bind_lmhi_receiver( WJP_BRIDGE_LMHIReceiver* lmhi_receiver ) { _lmhi_receiver = lmhi_receiver; }
    void bind_recv_buffer( WJP_MDsc_v mdsc ) { _recv_buffer = mdsc; }

#define _WJP_ASSERT_OR( c ) if( !(c) )
#define _WJP_INTER_MECH_INFO_ERR( s, e ) { info->err = ( (s) == 0 ? WJPErr_ConnReset : (e) ); } 
#define _WJP_INTER_MECH_CTX_INFO_ERR( s, e ) { context->info->err = ( (s) == 0 ? WJPErr_ConnReset : (e) ); } 
#define _WJP_ASSERT_AGENT( a ) { status = (a); if( status <= 0 ) return status; } 

    /**
     * @brief Push a resolver, send the head and wait for an ACK from the endpoint.
     * @attention This function sets: ALTERNATE | ACK_REQ | NOUN.
     */
    int WBCK_head( WJP_Head* head, WJP_WBCK_Info_RTG* info, int flags ) {
        head->set_alternate();
        head->set_ack_req();

        WJP_ScopedLock lock_resolvers{ &_mtx_resolvers };
        auto* res = _resolvers.push( WJP_WBCK_Resolver_RTG{ info: info, noun: _wbck_nouner.fetch_add( 1, WJP_MEM_ORD_RELEASE ) } );

        _WJP_ASSERT_OR( res != nullptr ) { info->err = WJPErr_QueueFull; return -1; }

        head->_dw1.noun = res->noun;

        WJP_ScopedLock lock_send{ &_mtx_send };
        int status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )head, sz: sizeof( WJP_Head ) }, flags, _arg );

        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Send ); _resolvers.trim(); return status; }

        return status;
    }

    /**
     * @brief Send the head to the endpoint.
     * @attention This function sets: ALTERNATE.
     */
    int BRST_head( WJP_Head* head, int flags ) {
        head->set_alternate();

        WJP_ScopedLock lock_send{ &_mtx_send };
        int status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )head, sz: sizeof( WJP_Head ) }, flags, _arg );

        return status;
    }

    /**
     * @brief Push a resolver, send the head with the payload and wait for an ACK from the endpoint.
     * @details If payload.addr is NULL, the device assumes that the payload immediately follows the head, thus calling send() once.
     * @attention This function sets: ACK_REQ | NOUN | SZ.
     * @attention This function resets: ALTERNATE.
     */
    int WBCK_payload( WJP_Head* head, WJP_WBCK_Info_RTG* info, WJP_MDsc_v payload, int flags ) {
        head->reset_alternate();
        head->set_ack_req();

        WJP_ScopedLock lock_resolvers{ &_mtx_resolvers };
        auto* res = _resolvers.push( WJP_WBCK_Resolver_RTG{ info: info, noun: _wbck_nouner.fetch_add( 1, WJP_MEM_ORD_RELEASE ) } );

        _WJP_ASSERT_OR( res != nullptr ) { info->err = WJPErr_QueueFull; return -1; }

        head->_dw1.noun = res->noun;
        head->_dw3.sz   = payload.sz;

        WJP_ScopedLock lock_send{ &_mtx_send };
        int status = 0;
        
        if( payload.addr == nullptr ) {
            status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )head, sz: payload.sz + ( int32_t )sizeof( WJP_Head ) }, flags, _arg );
            _WJP_ASSERT_OR( status == sizeof( WJP_Head ) + payload.sz ) goto l_err;
        } else {
            status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )&head, sz: sizeof( WJP_Head ) }, flags, _arg );
            _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) goto l_err;

            status = _inter_mech->send( payload, flags, _arg );
            _WJP_ASSERT_OR( status == payload.sz ) goto l_err;
        }
        goto l_ok;

    l_err:
        _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Send ); 
        _resolvers.trim(); 

    l_ok:
        return status;
    }

    /**
     * @brief Resolve one incoming packet.
     */
    int RSLV_head( WJP_RSLV_Info_RTG* info, int flags ) {
         _WJP_RSLV_Context_RTG _context, *context = &_context;
        context->info = info;
        context->flags = flags;
       
        int status = _inter_mech->recv( WJP_MDsc_v{ addr: &context->info->head, sz: sizeof( WJP_Head ) }, flags, _arg );

        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Recv ); return status; }

        context->recv_count = sizeof( WJP_Head );

        _WJP_ASSERT_OR( context->info->head.is_signed() ) { context->info->err = WJPErr_NotSigned; return -1; }

        if( context->info->head.is_lmhi() ) goto l_lmhi;

        switch( context->info->head._dw0.verb ) {
            case WJPVerb_Heart: {
                _WJP_ASSERT_AGENT( this->_resolve_heart( context ) );
            break; }

            case WJPVerb_Ack: 
            case WJPVerb_Nak: {
                _WJP_ASSERT_AGENT( this->_resolve_wbck( context ) );
            break; }

            l_lmhi: {
                _WJP_ASSERT_AGENT( this->_resolve_lmhi( context ) );
            break; }
        }

    l_end:
        return context->recv_count;
    }

    _WJP_forceinline int _resolve_heart( _WJP_RSLV_Context_RTG* context ) {
        _WJP_ASSERT_OR( context->info->head.is_alternate() ) {
            context->info->err = WJPErr_InvalidHctl;
            return -1;
        }

        WJP_Head head{};
        head._dw0.verb = WJPVerb_Ack;
        head._dw1.noun = context->info->head._dw1.noun;
        
        int status = this->BRST_head( &head, context->flags );

        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_CTX_INFO_ERR( status, WJPErr_Send ); return status; }

        context->info->sent_count = sizeof( WJP_Head );
        return 1;
    }

    _WJP_forceinline int _resolve_wbck( _WJP_RSLV_Context_RTG* context ) {
        auto* res = _resolvers.front();

        _WJP_ASSERT_OR( res != nullptr ) { context->info->err = WJPErr_NoResolver; return -1; }

        _WJP_ASSERT_OR( res->noun == context->info->head._dw1.noun ) { context->info->err = WJPErr_Sequence; return -1; }

        switch( context->info->head._dw1.hctl ) {
            case 0b00000000:
            case 0b01000000:

            case 0b10000000: {
                WJP_ScopedLock lock_resolvers{ &_mtx_resolvers }; _resolvers.pop(); goto l_end;
            }

            case 0b11000000: {

            }
        }

    l_end:
        return 1;
    }

    _WJP_forceinline int _resolve_lmhi( _WJP_RSLV_Context_RTG* context ) {
        _WJP_ASSERT_OR( _lmhi_receiver != nullptr ) {
            context->info->err = WJPErr_DeadEnd;
            return -1;
        }

        WJP_BRIDGE_LMHIReceiver::Params params;
        params.head_in = &context->info->head;

        int status = 0;

        if( !params.head_in->is_alternate() ) {
            _WJP_ASSERT_OR( params.head_in->_dw3.sz <= _recv_buffer.sz && params.head_in->_dw3.sz > 0 ) { context->info->err = WJPErr_Size; return -1; }

            status = _inter_mech->recv( WJP_MDsc_v{ addr: _recv_buffer.addr, sz: params.head_in->_dw3.sz }, context->flags, _arg );
            _WJP_ASSERT_OR( status == params.head_in->_dw3.sz ) { context->info->err = WJPErr_Recv; return status; }

            params.payload_in.addr = _recv_buffer.addr;
            params.payload_in.sz = params.head_in->_dw3.sz;
        }

        if( params.head_in->is_ack_req() ) {
            WJP_Head head_out{};
            params.head_out = &head_out;

            status = _lmhi_receiver->when_wbck( &params, _arg );

            head_out._dw0.verb = ( status == 0 ) ? WJPVerb_Ack : WJPVerb_Nak;
            head_out._dw1.noun = params.head_in->_dw1.noun;
            
            status = this->BRST_head( &head_out, context->flags );
            _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { context->info->err = WJPErr_Send; return status; }
        }

        return 1;
    }


    _WJP_forceinline int XO_drain( int flags = 0 ) {
        WJP_ScopedLock lock_send{ &_mtx_send };
        WJP_ScopedLock lock_resolvers{ &_mtx_resolvers };
   
        _resolvers.for_each( [] ( WJP_WBCK_Resolver_RTG* res ) -> void {
            res->info->err = WJPErr_Drained;
        } );
        _resolvers.clear();
       
        return _inter_mech->drain( flags, _arg );
    }

    /**
     * @brief Ping the endpoint.
     */
    _WJP_forceinline int XO_heart( WJP_WBCK_Info_RTG* info, int flags = 0 ) {
        WJP_Head head{};
        head._dw0.verb = WJPVerb_Heart;

        return this->WBCK_head( &head, info, flags );
    }

    /**
     * @brief Resolve one incoming packet.
     */
    _WJP_forceinline int XO_resolve( WJP_RSLV_Info_RTG* info, int flags = 0 ) {
        return this->RSLV_head( info, flags );
    }

    /**
     * @brief
     * @attention This function sets: LMHI
     */
    _WJP_forceinline int XO_LMHI_payload_ack_packed( WJP_WBCK_Info_RTG* info, WJP_Head*  head, int32_t payload_sz, int flags = 0 ) {
        head->set_lmhi();
        return this->WBCK_payload( head, info, WJP_MDsc_v{ addr: nullptr, sz: payload_sz  }, flags );
    }
};


#endif