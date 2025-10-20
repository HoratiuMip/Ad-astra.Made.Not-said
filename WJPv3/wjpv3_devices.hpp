#ifndef WJPV3_DEVICES_HPP
#define WJPV3_DEVICES_HPP
/*===== Warp Joint Protocol v3 - Devices - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Yes.
|
======*/
#include "wjpv3_core.hpp"


struct WJPInfo_Send {
    WJPErr_   err   = WJPErr_None;
};


struct WJPDevice_Euclid {
    inline static const int _PHASE_BUFFER_SIZE = 18;
    inline static const unsigned char _PHASE_BUFFER[ _PHASE_BUFFER_SIZE ] = {
        'W', 'J', 'P', '3'
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        'W', 'J', 'P', '3'
    };

    WJP_InterMech*      _inter_mech      = nullptr;
    WJP_MDsc            _recv_mdsc     = {};

    WJP_LMHIReceiver*   _lmhi_receiver   = nullptr;

    void bind_inter_mech( WJP_InterMech* inter_mech_ ) { _inter_mech = inter_mech_; }
    void bind_recv_buffer( WJP_MDsc mdsc_ ) { _recv_mdsc = mdsc_; }
    void bind_lmhi_receiver( WJP_LMHIReceiver* lmhi_receiver_ ) { _lmhi_receiver = lmhi_receiver_; }

#define _WJP_ASSERT_OR( c ) if( !(c) )
#define _WJP_INTER_MECH_INFO_ERR( s, e ) { info->err = ( (s) == 0 ? WJPErr_Reset : (e) ); } 
#define _WJP_INTER_MECH_CTX_INFO_ERR( s, e ) { context->info->err = ( (s) == 0 ? WJPErr_Reset : (e) ); } 
#define _WJP_ASSERT_AGENT( a ) { status = (a); if( status <= 0 ) return status; } 

    int send_lmhi_head_exp( WJPInfo_Send* info, int16_t ACT, int32_t ARG, int32_t N ) {
        WJP_Head head = {}; 

        head._dw1.ACT = ACT; head._dw2.ARG = ARG; head._dw3.N = N;
        head.reset_payload();
        head.set_lmhi();

        int result = _inter_mech->send( { ( char* )&head, sizeof( WJP_Head ) } );
        _WJP_ASSERT_OR( sizeof( WJP_Head ) == result ) {
            _WJP_INTER_MECH_INFO_ERR( result, WJPErr_Send );
            return result;
        }

        return result;
    }

    int send_lmhi_payload_pck( WJPInfo_Send* info, WJP_Head* head, int32_t N = 0 ) {
        if( N > 0 ) head->_dw3.N = N;

        head->set_payload();
        head->set_lmhi();

        int acc_size = sizeof( WJP_Head ) + head->_dw3.N;

        int result = _inter_mech->send( { ( char* )head, acc_size } );
        _WJP_ASSERT_OR( acc_size == result ) {
            _WJP_INTER_MECH_INFO_ERR( result, WJPErr_Send );
            return result;
        }

        return result;
    }
    

    /**
     * @brief Resolve one incoming packet.
     */
    int RSLV_head( WJP_RSLV_Info_RTG* info, int flags ) {
         _WJP_RSLV_Context_RTG _context, *context = &_context;
        context->info = info;
        context->flags = flags;
       
        int status = 0;
        
        if( _phase_lock == 0 ) [[likely]] {
            status = _inter_mech->recv( WJP_MDsc_v{ addr: &context->info->head, sz: sizeof( WJP_Head ) }, flags, _arg );
            _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Recv ); return status; }
        } else {
            int idx = 0;

            while( idx < _PHASE_BUFFER_SIZE ) {
                unsigned char byte;
                status = _inter_mech->recv( WJP_MDsc_v{ addr: ( void* )&byte, sz: 1 }, flags, _arg );
                _WJP_ASSERT_OR( status == 1 ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Recv ); return status; }

                _WJP_ASSERT_OR( --_phase_lock > 0 ) { info->err = WJPErr_PhaseLock; return -1; } 

                if( byte != _PHASE_BUFFER[ idx ] ) { idx = 0; continue; }
                ++idx;
            }

            _phase_lock = 0;
            return 1;
        }
   
        context->recv_count = sizeof( WJP_Head );

        _WJP_ASSERT_OR( context->info->head.is_signed() ) { context->info->err = WJPErr_NotSigned; return -1; }

        if( context->info->head.is_lmhi() ) goto l_lmhi;

        switch( context->info->head._dw0.verb ) {
            case WJPVerb_Heart: {
                _WJP_ASSERT_AGENT( this->_resolve_heart( context ) );
            break; }

            case WJPVerb_PhaseLock: {
                _WJP_ASSERT_AGENT( this->_resolve_phase_lock( context ) );
            break; }

            case WJPVerb_Ack: 
            case WJPVerb_Nak: {
                _WJP_ASSERT_AGENT( this->_resolve_ack( context ) );
            break; }

            l_lmhi: {
                _WJP_ASSERT_AGENT( this->_resolve_lmhi( context ) );
            break; }
        }

    l_end:
        return context->recv_count;
    }

    _WJP_forceinline int _resolve_heart( _WJP_RSLV_Context_RTG* context ) {
        _WJP_ASSERT_OR( context->info->head.is_alternate() ) { context->info->err = WJPErr_InvalidHctl; return -1; }

        WJP_Head head{};
        head._dw0.verb = WJPVerb_Ack;
        head._dw1.noun = context->info->head._dw1.noun;
        
        int status = this->BRST_head( &head, nullptr, context->flags );
        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_CTX_INFO_ERR( status, WJPErr_Send ); return status; }

        context->info->sent_count = sizeof( WJP_Head );
        return 1;
    }

    _WJP_forceinline int _resolve_phase_lock( _WJP_RSLV_Context_RTG* context ) {
        _WJP_ASSERT_OR( context->info->head.is_alternate() ) { context->info->err = WJPErr_InvalidHctl; return -1; }

        int status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )_PHASE_BUFFER, sz: _PHASE_BUFFER_SIZE }, context->flags, _arg );
        _WJP_ASSERT_OR( status == _PHASE_BUFFER_SIZE ) { _WJP_INTER_MECH_CTX_INFO_ERR( status, WJPErr_Send ); return status; }

        context->info->sent_count = _PHASE_BUFFER_SIZE;
        return 1;
    }

    _WJP_forceinline int _resolve_ack( _WJP_RSLV_Context_RTG* context ) {
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
            params.payload_in.sz   = params.head_in->_dw3.sz;
        }

        if( params.head_in->_dw0.verb == WJPVerb_Ack || params.head_in->_dw0.verb == WJPVerb_Nak ) {
            auto* res = _resolvers.front();
            _WJP_ASSERT_OR( res != nullptr ) { context->info->err = WJPErr_NoResolver; return -1; }
            _WJP_ASSERT_OR( res->noun == params.head_in->_dw1.noun ) { context->info->err = WJPErr_Sequence; return -1; }

            status = _lmhi_receiver->when_ack( &params, _arg );

            WJP_ScopedLock lock_resolvers{ &_mtx_resolvers }; _resolvers.pop();
            goto l_end;
        }

        if( params.head_in->is_ack_req() ) {
            WJP_Head head_out{};
            params.head_out = &head_out;

            status = _lmhi_receiver->when_wbck( &params, _arg );

            head_out._dw0.verb = ( status == 0 ) ? WJPVerb_Ack : WJPVerb_Nak;
            head_out._dw1.noun = params.head_in->_dw1.noun;
            //head_out.set_lmhi();
            
            status = this->BRST_head( &head_out, nullptr, context->flags );
            _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { context->info->err = WJPErr_Send; return status; }
            goto l_end;
        } else {
            status = _lmhi_receiver->when_brst( &params, _arg );
            goto l_end;
        }

    l_end:
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

    _WJP_forceinline int XO_phase_lock( WJP_BRST_Info_RTG* info, int phases = 1500, int flags = 0 ) {
        WJP_Head head{};
        head._dw0.verb = WJPVerb_PhaseLock;

        _phase_lock = phases;

        int status = this->BRST_head( &head, info, flags );
        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Send ); _phase_lock = 0; }

        return status;
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
     * @brief LMHI a packed payload for which an ACK is required.
     * @attention This function sets: LMHI
     */
    _WJP_forceinline int XO_LMHI_payload_ack_packed( WJP_WBCK_Info_RTG* info, WJP_Head*  head, int32_t payload_sz, int flags = 0 ) {
        head->set_lmhi();
        return this->WBCK_payload( head, info, WJP_MDsc_v{ addr: nullptr, sz: payload_sz  }, flags );
    }

    /**
     * @brief LMHI a packed payload for which an ACK is NOT required.
     * @attention This function sets: LMHI
     */
    _WJP_forceinline int XO_LMHI_payload_nak_packed( WJP_BRST_Info_RTG* info, WJP_Head*  head, int32_t payload_sz, int flags = 0 ) {
        head->set_lmhi();
        return this->BRST_payload( head, info, WJP_MDsc_v{ addr: nullptr, sz: payload_sz  }, flags );
    }
};


#endif