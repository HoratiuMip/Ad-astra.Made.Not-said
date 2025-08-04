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

    void bind_inter_mech( WJP_BRIDGE_InterMech* inter_mech ) { _inter_mech = inter_mech; }
    void bind_resolvers_mdsc( WJP_MDsc< WJP_WBCK_Resolver_RTG > mdsc ) { _resolvers._mdsc = mdsc; }

#define _WJP_ASSERT_OR( c ) if( !(c) )
#define _WJP_INTER_MECH_INFO_ERR( s, e ) { info->err = ( (s) == 0 ? WJPErr_ConnReset : (e) ); } 
#define _WJP_INTER_MECH_CTX_INFO_ERR( s, e ) { context->info->err = ( (s) == 0 ? WJPErr_ConnReset : (e) ); } 
#define _WJP_ASSERT_AGENT( a ) { status = (a); if( status <= 0 ) return status; } 

    int WBCK_head( WJP_Head* head, WJP_WBCK_Info_RTG* info, int flags ) {
        head->set_alternate();

        WJP_ScopedLock lock_resolvers{ &_mtx_resolvers };
        auto* res = _resolvers.push( WJP_WBCK_Resolver_RTG{ info: info, noun: _wbck_nouner.fetch_add( 1, WJP_MEM_ORD_RELEASE ) } );

        _WJP_ASSERT_OR( res != nullptr ) { info->err = WJPErr_QueueFull; return -1; }

        WJP_ScopedLock lock_send{ &_mtx_send };
        int status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )head, sz: sizeof( WJP_Head ) }, flags, _arg );

        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Send ); _resolvers.trim(); return status; }

        return status;
    }

    int RSLV_one( WJP_RSLV_Info_RTG* info, int flags ) {
         _WJP_RSLV_Context_RTG _context, *context = &_context;
        context->info = info;
        context->flags = flags;
       
        int status = _inter_mech->recv( WJP_MDsc_v{ addr: &context->info->head, sz: sizeof( WJP_Head ) }, flags, _arg );

        _WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) { _WJP_INTER_MECH_INFO_ERR( status, WJPErr_Recv ); return status; }

        context->recv_count = sizeof( WJP_Head );

        _WJP_ASSERT_OR( context->info->head.is_signed() ) { context->info->err = WJPErr_NotSigned; return -1; }

        switch( context->info->head._dw0.verb ) {
            case WJPVerb_Heart: {
                _WJP_ASSERT_AGENT( this->_resolve_heart( context ) );
            break; }

            case WJPVerb_Ack: {
                _WJP_ASSERT_AGENT( this->_resolve_wbck( context ) );
            break; }
        }

    l_end:
        return context->recv_count;
    }

    _WJP_forceinline int _resolve_heart( _WJP_RSLV_Context_RTG* context ) {
        WJP_Head head{};
        head._dw0.verb = WJPVerb_Ack;
        head._dw1.noun = context->info->head._dw1.noun;
        head.set_alternate();
        
        WJP_ScopedLock lock_send{ &_mtx_send };
        int status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )&head, sz: sizeof( WJP_Head ) }, context->flags, _arg );
        lock_send.release();

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


    _WJP_forceinline int XO_heart( WJP_WBCK_Info_RTG* info, int flags = 0 ) {
        WJP_Head head{};
        head._dw0.verb = WJPVerb_Heart;

        return this->WBCK_head( &head, info, flags );
    }

    _WJP_forceinline int XO_resolve_one( WJP_RSLV_Info_RTG* info, int flags = 0 ) {
        return this->RSLV_one( info, flags );
    }
};


#endif