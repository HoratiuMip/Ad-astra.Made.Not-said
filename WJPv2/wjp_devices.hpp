#pragma once
/*===== Warp Joint Protocol v2 - Devices - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Yes.
|
======*/
#include "wjp_core.hpp"
#include "wjp_bridges.hpp"
#include "wjp_structs.hpp"

#include "wjp_bridges_RTG.hpp"


enum WJPSendMethod_ {
    /* Make a call to send for each source. */
    WJPSendMethod_Direct,

    /* Copy on the stack from all the sources and then make one call to send. */
    WJPSendMethod_Stack,

    /* Let WJP decide, based on the underlying system and the count of bytes to send. */
    WJPSendMethod_Auto
};

/*
|>  DEVICE ======
*/
#define WJP_NAKR_MAX_SIZE 32

#define _WJP_ASSERT( c, r ) if( !( c ) ) return ( r );
#define _WJP_ASSERT_INFO( c, e, r ) if( !( c ) ) { info->err = e; return ( r ); }
#define _WJP_ASSERT_CTX_INFO( c, e, r ) if( !( c ) ) { context->info->err = e; return ( r ); }


struct WJP_RESOLVE_RECV_INFO {
    WJP_HEAD      recv_head    = {};
    int32_t       sent_count   = 0;
    WJPErr_       err          = WJPErr_None;
    const char*   nakr         = nullptr;
};

struct WJP_WAIT_BACK_INFO {
    _WJP_INTERLOCKED_BOOL   resolved                        = { _flag: false };
    int32_t                 recv_count                      = 0;
    WJPErr_                 err                             = WJPErr_None;
    char                    nakr[ WJP_NAKR_MAX_SIZE + 1 ]   = { '\0' };

    _WJP_forceinline bool ackd( void ) { return nakr[ 0 ] == '\0'; }
};

struct _WJP_RESOLVER {
#if defined WJP_ENVIRONMENT_AVR
    _WJP_RESOLVER() = default;
    _WJP_RESOLVER(
        int16_t s, WJP_BUFFER d, WJP_WAIT_BACK_INFO* i
    ) : seq{ s }, dst{ d }, info{ i } {}
#endif

    int16_t               seq    = 0;
    WJP_BUFFER            dst    = {};
    WJP_WAIT_BACK_INFO*   info   = nullptr;
};

struct _WJP_RECV_CONTEXT {
    void*                    data         = nullptr;
    int32_t                  sz           = 0;
    WJP_RESOLVE_RECV_INFO*   info         = nullptr;
    int32_t                  recv_count   = 0;
};

/**
 * @brief Called by WJP when a new sequence for a head is required. Usually an interlocked increment does the job.
 * @returns The sequence.
 */
#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    typedef   std::function< int16_t( void ) >   WJP_SEQ_ACQ_FUNC;
#else
    typedef   int16_t ( *WJP_SEQ_ACQ_FUNC )( void );
#endif

#if defined( WJP_ENVIRONMENT_MINGW ) || defined( WJP_ENVIRONMENT_RTOS )
    typedef   std::function< int( _WJP_RECV_CONTEXT* ) > WJP_RECV_FWD_FUNC;
#else
    typedef   int ( *WJP_RECV_FWD_FUNC )( _WJP_RECV_CONTEXT* );
#endif

#define WJP_RECV_FWD_LAMBDA ( _WJP_RECV_CONTEXT* context ) -> int

struct WJP_DEVICE {
    WJP_SRWRAP                             _srwrap      = {};
    WJP_BUFFER                             _recv_buf    = {};

    WJP_SEQ_ACQ_FUNC                       _seq_acq     = nullptr;
    WJP_RECV_FWD_FUNC                      _recv_fwd    = nullptr;

    WJP_QGSTBL                             _qgstbl      = {};
    WJP_IBRSTBL                            _ibrstbl     = {};

    _WJP_MUTEX                             _res_mtx     = {};
    _WJP_MUTEX                             _snd_mtx     = {};

    _WJP_CIRCULAR_QUEUE< _WJP_RESOLVER >   _resolvers   = {};

/*
|>  BINDINGS ======
*/
    void bind_recv_buf( const WJP_BUFFER& buf ) { _recv_buf = buf; }
    void bind_resolver_buf( const WJP_BUFFER_EX< _WJP_RESOLVER > buf ) { _resolvers.reset_buffer( buf ); }

    void bind_srwrap( const WJP_SRWRAP& srwrap ) { _srwrap = srwrap; }

    void bind_seq_acq( const WJP_SEQ_ACQ_FUNC& seq_acq ) { _seq_acq = seq_acq; }
    void bind_recv_fwd( const WJP_RECV_FWD_FUNC& recv_fwd ) { _recv_fwd = recv_fwd; }

    void bind_qgstbl( const WJP_QGSTBL& qgstbl ) { _qgstbl = qgstbl; }
    void bind_ibrstbl( const WJP_IBRSTBL& ibrstbl ) { _ibrstbl = ibrstbl; }

/*
|>  CONTROL ======
*/
    int open( int flags ) {
        _WJP_ASSERT( _srwrap.recv != nullptr && _srwrap.send != nullptr, -1 );
        _WJP_ASSERT( _recv_buf.addr != nullptr && _recv_buf.sz >= 3, -2 );
        _WJP_ASSERT( _seq_acq != nullptr, -3 );

        _recv_buf[ 0 ] = _recv_buf[ _recv_buf.sz - 1 ] = ( char )_seq_acq();

        return 0;
    }

    int close( int flags ) {
        _resolvers.clear();
        return 0;
    }


    int32_t _sizeof_send_n_descs( WJP_CBUFFER* descs, int count ) {
        int sz = 0;
        for( int idx = 0; idx < count; ++idx ) sz += descs[ idx ].sz;
        return sz;
    }

    template< bool _HAS_HEAD = WJP_NO_HEAD >
    int _send_n( WJP_CBUFFER* descs, int count, WJPOp_ op, int16_t seq, int flags, WJPSendMethod_ method ) {
        int ret = 0;

        switch( method ) {
        l_direct: case WJPSendMethod_Direct: {        
                if constexpr( WJP_NO_HEAD == _HAS_HEAD ) {
                    WJP_HEAD head;
                    head._dw0.op  = op;
                    head._dw1.seq = seq;
                    head._dw3.sz  = _sizeof_send_n_descs( descs, count );

                    int crt_ret = _srwrap.send( WJP_CBUFFER{ addr: &head, sz: sizeof( head ) }, flags );
                    _WJP_ASSERT( crt_ret == sizeof( head ), crt_ret <= 0 ? crt_ret : -1 );
                    ret += crt_ret;
                }

                for( int idx = 0; idx < count; ++idx ) {
                    int crt_ret = _srwrap.send( WJP_CBUFFER{ addr: descs[ idx ].addr, sz: descs[ idx ].sz }, flags );
                    _WJP_ASSERT( crt_ret == descs[ idx ].sz, crt_ret <= 0 ? crt_ret : -1 );
                    ret += crt_ret;
                }
            break; }

        l_stack: case WJPSendMethod_Stack: {
                int tot_sz = _sizeof_send_n_descs( descs, count ) + _HAS_HEAD ? 0 : sizeof( WJP_HEAD );

                char buffer[ tot_sz ];
                
                if constexpr( WJP_NO_HEAD == _HAS_HEAD ) {
                    ( ( WJP_HEAD* )buffer )->_dw0.op  = op;
                    ( ( WJP_HEAD* )buffer )->_dw1.seq = seq;
                    ( ( WJP_HEAD* )buffer )->_dw3.sz  = tot_sz - sizeof( WJP_HEAD );
                    tot_sz = sizeof( WJP_HEAD );
                } else {
                    tot_sz = 0;
                }

                for( int idx = 0; idx < count; ++idx ) {
                    memcpy( buffer + tot_sz, descs[ idx ].addr, descs[ idx ].sz );
                    tot_sz += descs[ idx ].sz;
                }
                
                ret = _srwrap.send( WJP_CBUFFER{ addr: buffer, sz: tot_sz }, flags );
                _WJP_ASSERT( ret == tot_sz, ret <= 0 ? ret : -1 );
            break; }

            case WJPSendMethod_Auto: {
                if( descs == nullptr )
                    goto l_direct;
                else if( sizeof( WJP_HEAD ) + _sizeof_send_n_descs( descs, count ) <= 64  )
                    goto l_stack;
                else 
                    goto l_direct;
            break; }
        }

        return ret;
    } 
    
    template< bool _HAS_HEAD = WJP_NO_HEAD >
    int _send( const void* src_, int32_t sz_, WJPOp_ op, int16_t seq, int flags, WJPSendMethod_ method ) {
        WJP_CBUFFER desc{ addr: src_, sz: sz_ };
        if( src_ != nullptr )
            return _send_n< _HAS_HEAD >( &desc, 1, op, seq, flags, method );
        return _send_n< _HAS_HEAD >( nullptr, 0, op, seq, flags, method );
    }


#define _WJP_RR_ASSERT_RECV( r, t, e ) { int _rret = ( r );\
    if( _rret < 0 ) { context->info->err = e; return _rret; }\
    if( _rret == 0 ) { context->info->err = WJPErr_ConnReset; return 0; }\
    context->recv_count += _rret;\
    if( _rret != ( t ) ) { context->info->err = WJPErr_CountRecv; return -1; } }

#define _WJP_RR_ASSERT__SEND( s, e ) { int _sret = ( s );\
    if( _sret < 0 ) { context->info->err = e; return _sret; }\
    if( _sret == 0 ) { context->info->err = WJPErr_ConnReset; return 0; }\
    context->info->sent_count += _sret; }

#define _WJP_RR_ASSERT_AGENT( a ) { int _aret = ( a ); if( _aret <= 0 ) return _aret; } 

#define _WJP_RR_NAK_IF( c, r ) { if( c ) { return this->_nak( context, r ); } }

    _WJP_forceinline int _resolve_heart( _WJP_RECV_CONTEXT* context ) {
        _WJP_RR_ASSERT__SEND( 
            _send<>( nullptr, 0, WJPOp_Ack, context->info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ), 
            WJPErr_Send 
        );

        return 1;
    }

    _WJP_forceinline int _nak( _WJP_RECV_CONTEXT* context, const char* nakr ) {
        context->info->nakr = nakr;
        _WJP_RR_ASSERT__SEND(
            _send<>( nakr, nakr ? strlen( nakr ) + 1 : 0, WJPOp_Nak, context->info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
            WJPErr_Send 
        );
        return 1;
    }

    _WJP_forceinline int _resolve_qgetset( _WJP_RECV_CONTEXT* context ) {
        _WJP_RR_NAK_IF( 
            context->info->recv_head._dw3.sz == 0 || context->info->recv_head._dw3.sz > _recv_buf.sz, 
            "WJP_NAKR_BAD_SIZE"
        );

        _WJP_RR_ASSERT_RECV(
            _srwrap.recv( WJP_BUFFER{ addr: _recv_buf.addr, sz: context->info->recv_head._dw3.sz }, 0 ), 
            context->info->recv_head._dw3.sz, 
            WJPErr_Recv
        );

        char* delim = _recv_buf - 1; while( *++delim != '\0' ) {
            _WJP_RR_NAK_IF( delim - _recv_buf >= context->info->recv_head._dw3.sz - 1, "WJP_NAKR_BAD_STRING" );
        }

        WJP_QGSTBL_ENTRY* entry = _qgstbl.query( _recv_buf );
        _WJP_RR_NAK_IF( entry == nullptr, "WJP_NAKR_NO_ENTRY" );

        int32_t args_sz = context->info->recv_head._dw3.sz - ( delim - _recv_buf + 1 );

        if( context->info->recv_head._dw0.op == WJPOp_QSet ) goto l_qset;

    l_qget:
        if( entry->qget_func != nullptr ) {
            char buffer[ entry->sz ];

            const char* nakr = entry->qget_func( buffer, WJP_BUFFER{ addr: delim + 1, sz: args_sz } );
            _WJP_RR_NAK_IF( nakr != nullptr, nakr );

            _WJP_RR_ASSERT__SEND( 
                _send<>( buffer, entry->sz, WJPOp_Ack, context->info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
                WJPErr_Send
            );

        } else if( entry->src != nullptr ) {
            _WJP_RR_ASSERT__SEND( 
                _send<>( entry->src, entry->sz, WJPOp_Ack, context->info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
                WJPErr_Send
            );

        } else {
            _WJP_RR_NAK_IF( true, "WJP_NAKR_NO_PROCEDRE" );
        }
        
        return 1;

    l_qset:
        _WJP_RR_NAK_IF( entry->read_only, "WJP_NAKR_READ_ONLY" );
        _WJP_RR_NAK_IF( entry->sz != args_sz, "WJP_NAKR_BAD_SIZE" );

        if( entry->qset_func != nullptr ) {
            const char* nakr = entry->qset_func( WJP_BUFFER{ addr: delim + 1, sz: args_sz } );
            _WJP_RR_NAK_IF( nakr != nullptr, nakr );

        } else if( entry->src != nullptr ) {
            memcpy( entry->src, delim + 1, args_sz );

        } else {
            _WJP_RR_NAK_IF( true, "WJP_NAKR_NO_PROCEDRE" );
        }

        _WJP_RR_ASSERT__SEND( 
            _send<>( nullptr, 0, WJPOp_Ack, context->info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
            WJPErr_Send
        );

        return 1;
    }

    _WJP_forceinline int _resolve_acknak( _WJP_RECV_CONTEXT* context ) {
        _WJP_ASSERT_CTX_INFO( context->info->recv_head._dw3.sz >= 0, WJPErr_Size, -1 );
        _WJP_ASSERT_CTX_INFO( !_resolvers.is_empty(), WJPErr_NoResolver, -1 );

        _WJP_RESOLVER& resolver = _resolvers.front();

        _WJP_ASSERT_CTX_INFO( context->info->recv_head._dw1.seq == resolver.seq, WJPErr_Sequence, -1 );

        WJP_WAIT_BACK_INFO* wb_info = resolver.info;

        if( context->info->recv_head._dw0.op == WJPOp_Nak ) {
            //_WJP_ASSERT_INFO( info->recv_head._dw3.sz <= WJP_NAKR_MAX_SIZE, WJP_ERR_NAKR_SIZE, -1 );
            _WJP_RR_ASSERT_RECV(
                _srwrap.recv( WJP_BUFFER{ addr: wb_info->nakr, sz: context->info->recv_head._dw3.sz }, 0 ), 
                context->info->recv_head._dw3.sz, 
                WJPErr_Recv
            );
            goto l_release_resolver;
        }

        if( resolver.dst.addr == nullptr || context->info->recv_head._dw3.sz == 0 ) goto l_release_resolver;

        _WJP_ASSERT_CTX_INFO( resolver.dst.sz >= context->info->recv_head._dw3.sz, WJPErr_Size, -1 );
        _WJP_RR_ASSERT_RECV(
            _srwrap.recv( WJP_BUFFER{ addr: resolver.dst.addr, sz: context->info->recv_head._dw3.sz }, 0 ), 
            context->info->recv_head._dw3.sz, 
            WJPErr_Recv
        );

        wb_info->recv_count += context->info->recv_head._dw3.sz;

    l_release_resolver:
        wb_info->recv_count += sizeof( WJP_HEAD );
        
        _resolvers.pop();
        
        wb_info->resolved.set( _WJP_MEM_ORD_RELEASE );
        wb_info->resolved.sig_one();
    
        return 1;
    }

    _WJP_forceinline int _resolve_iburst( _WJP_RECV_CONTEXT* context ) {
        _WJP_ASSERT_CTX_INFO( context->info->recv_head._dw3.sz >= 0, WJPErr_Size, -1 );

        WJP_IBRSTBL_ENTRY* entry = _ibrstbl[ context->info->recv_head._dw1.seq ];
        _WJP_ASSERT_CTX_INFO( entry != nullptr, WJPErr_NoEntry, -1 );
        _WJP_ASSERT_CTX_INFO( context->info->recv_head._dw3.sz <= entry->sz, WJPErr_Size, -1 );

        _WJP_RR_ASSERT_RECV(
            _srwrap.recv( WJP_BUFFER{ addr: entry->mem, sz: context->info->recv_head._dw3.sz }, 0 ), 
            context->info->recv_head._dw3.sz, 
            WJPErr_Recv
        );

        return 1;
    }

    _WJP_forceinline int _resolve_iburst_control( _WJP_RECV_CONTEXT* context ) {
        _WJP_ASSERT_CTX_INFO( context->info->recv_head.is_alternate(), WJPErr_InvalidHctl, -1 );

        WJP_IBRSTBL_ENTRY* entry = _ibrstbl[ context->info->recv_head._dw3.sz ];
        _WJP_ASSERT_CTX_INFO( entry != nullptr, WJPErr_NoEntry, -1 );

        switch( ( WJPIbrstCtrlEngage_ )( context->info->recv_head._dw1.octl & WJP_IBRST_CTRL_ENGAGE_MSK ) ) {
            case WJPIbrstCtrlEngage_Terminate: {
                entry->_status = WJPIBrstStatus_Disengaged; 
            goto l_ack; }

            case WJPIbrstCtrlEngage_AutoThrottle:  {
                entry->_status = WJPIBrstStatus_Engaged;
            goto l_ack; }

            default: _WJP_ASSERT_CTX_INFO( false, WJPErr_InvalidOctl, -1 ); 
        }

    l_ack:
        _WJP_RR_ASSERT__SEND( 
            _send<>( nullptr, 0, WJPOp_Ack, context->info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
            WJPErr_Send
        );

        return 1;
    }

    int resolve_recv( WJP_RESOLVE_RECV_INFO* info ) {
        _WJP_RECV_CONTEXT _context, *context = &_context;
        context->info = info;
        
        _WJP_RR_ASSERT_RECV(
            _srwrap.recv( WJP_BUFFER{ addr: &context->info->recv_head, sz: sizeof( WJP_HEAD ) }, 0 ), 
            sizeof( WJP_HEAD ), 
            WJPErr_Recv
        );

        _WJP_ASSERT_CTX_INFO( context->info->recv_head.is_signed(), WJPErr_NotSigned, -1 );

        WJP_RECV_FWD_FUNC recv_fwd = _recv_fwd;
        if( recv_fwd && recv_fwd( context ) != 0 ) goto l_end;

        switch( context->info->recv_head._dw0.op ) {
            case WJPOp_Heart: {
                _WJP_RR_ASSERT_AGENT( this->_resolve_heart( context ) );
            break; }

            case WJPOp_QGet: [[fallthrough]];
            case WJPOp_QSet: {
                _WJP_RR_ASSERT_AGENT( this->_resolve_qgetset( context ) );
            break; }

            case WJPOp_Nak: [[fallthrough]];
            case WJPOp_Ack: {
                _WJP_RR_ASSERT_AGENT( this->_resolve_acknak( context ) );
            break; }

            case WJPOp_IBurst: {
                _WJP_RR_ASSERT_AGENT( this->_resolve_iburst( context ) );
            break; }
            case WJPOp_IBurstCtl: {
                _WJP_RR_ASSERT_AGENT( this->_resolve_iburst_control( context ) );
            break; }
        }

    l_end:
        return context->recv_count;
    }

#define _WJP_WB_ASSERT__SEND( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? WJPErr_ConnReset : e; return _sret; } }
    int wait_back( 
        WJP_WAIT_BACK_INFO* info, 
        WJPOp_              op, 
        const WJP_CBUFFER   src,
        const WJP_BUFFER    dst,
        WJPSendMethod_      method
    ) {
         _WJP_SCOPED_LOCK res_lock{ &_res_mtx };

        _WJP_RESOLVER* resolver = _resolvers.push( _WJP_RESOLVER{
            seq:  _seq_acq(),
            dst:  dst,
            info: info
        } );

        _WJP_ASSERT_INFO( resolver != nullptr, WJPErr_QueueFull, -1 );

        _WJP_SCOPED_LOCK snd_lock{ &_snd_mtx };
        res_lock.unlock();

        int ret = _send<>( src.addr, src.sz, op, resolver->seq, 0, method );
        _WJP_WB_ASSERT__SEND( ret, WJPErr_Send );

        return ret;
    }

    int wait_back_head_xchg(
        WJP_WAIT_BACK_INFO* info,
        WJP_HEAD*           head,
        int                 flags
    ) {
        _WJP_SCOPED_LOCK res_lock{ &_res_mtx };

        _WJP_RESOLVER* resolver = _resolvers.push( _WJP_RESOLVER{
            seq:  _seq_acq(),
            dst:  WJP_BUFFER{ addr: nullptr, sz: 0 },
            info: info
        } );

        _WJP_ASSERT_INFO( resolver != nullptr, WJPErr_QueueFull, -1 );

        head->_dw1.seq = resolver->seq;

        _WJP_SCOPED_LOCK snd_lock{ &_snd_mtx };

        int ret = _srwrap.send( WJP_CBUFFER{ addr: head, sz: sizeof( WJP_HEAD ) }, flags );
        _WJP_WB_ASSERT__SEND( ret, WJPErr_Send );

        return ret;
    }

    void force_waiting_resolvers() {
        _WJP_SCOPED_LOCK lock{ &_res_mtx };

        _resolvers.for_each( [] ( _WJP_RESOLVER& resolver ) -> void {
            strcpy( resolver.info->nakr, WJP_err_strs[ WJPErr_Forced ] );
            resolver.info->err = WJPErr_Forced;
            resolver.info->resolved.set( _WJP_MEM_ORD_RELEASE );
            resolver.info->resolved.sig_all();
        } );
        _resolvers.clear();
    }

    bool breach( void ) {
        const char str[] = "WJP_INTENTIONAL_BREACH";
        return _srwrap.send( WJP_CBUFFER{ addr: str, sz: strlen( str ) }, 0 ) == strlen( str );
    }

/*
|>  WRAPPERS ======
*/
    //_WJP_forceinline int qget( WJP_WAIT_BACK_INFO* info, WJP_BUFFER )

    int ibrst_ctl( WJP_WAIT_BACK_INFO* info, int16_t index, uint8_t ctl, int flags ) {
        WJP_HEAD head = {};

        head.set_alternate();
        head._dw0.op = WJPOp_IBurstCtl;
        head._dw1.octl = ctl;
        head._dw3.sz = index;

        return this->wait_back_head_xchg( info, &head, flags );
    }

    inline int ibrst_terminate( WJP_WAIT_BACK_INFO* info, int16_t index, int flags ) {
        return this->ibrst_ctl( info, index, ( int16_t )WJPIbrstCtrlEngage_Terminate, flags );
    }

    inline int ibrst_auto_throttle_engage( WJP_WAIT_BACK_INFO* info, int16_t index, int flags ) {
        return this->ibrst_ctl( info, index, ( int16_t )WJPIbrstCtrlEngage_AutoThrottle, flags );
    }

    int ibrst( int16_t index, int flags ) {
        WJP_IBRSTBL_ENTRY* entry = &_ibrstbl.entries[ index ];

        if( entry->_status == WJPIBrstStatus_Disengaged ) return 0;
        if( entry->_status > 0 ) --entry->_status;

        if( entry->mem != nullptr ) {
            return -1;
        } else if( entry->out_fnc != nullptr ) {
            if( entry->packed )
                return _srwrap.send( WJP_CBUFFER{ addr: entry->out_fnc(), sz: entry->sz }, flags );
            else
                return -1;
        } 

        return -1;
    }

/*
|>  XO ======
*/
    int XO_phase_lock( WJP_WAIT_BACK_INFO* info, int flags = 0 ) {
        
    }

    int XO_heart( WJP_WAIT_BACK_INFO* info, int flags = 0 ) {
        WJP_HEAD head = {};

        head.set_alternate();
        head._dw0.op = WJPOp_Heart;

        return this->wait_back_head_xchg( info, &head, flags );
    }

};


struct WJP_WACK_Info_RTG {
    WJP_Interlocked< bool >   resolved     = { false };
    int                       recv_count   = 0;
    WJPErr_                   err_this     = WJPErr_None;
    WJPErr_                   err_that     = WJPErr_None;
};

struct WJP_WACK_Resolver_RTG {
#if deinfed( _WJP_SEMANTICS_AGGREGATES_REQUIRE_CONSTRUCTOR )
    WJP_WACK_Resolver_RTG() = default;

    WJP_WACK_Resolver_RTG( WJP_WACK_Info_RTG* info_, int16_t noun_ )
    : info{ info_ }, noun{ noun_ } {}
#endif

    WJP_WACK_Info_RTG*    info   = nullptr;
    int16_t               noun   = 0;
};

struct WJP_DEVICE_Cat_1_RTG {
    WJP_BRIDGE_InterMech*                        _inter_mech      = nullptr;

    WJP_Interlocked< int16_t >                   _wack_nouner     = { 0 };

    WJP_CircularQueue< WJP_WACK_Resolver_RTG >   _resolvers       = {};

    WJP_Mutex                                    _mtx_resolvers   = {};
    WJP_Mutex                                    _mtx_send        = {};

    void*                                        _arg             = nullptr;

#define WJP_ASSERT_OR( c ) if( !(c) )
#define WJP_INTER_MECH_INFO_ERR( s, e ) { info->err_this = ( (s) == 0 ? WJPErr_ConnReset : (e) ); } 

    int WACK_alternate( WJP_Head* head, WJP_WACK_Info_RTG* info, int flags ) {
        head->set_alternate();

        WJP_ScopedLock lock_resolvers{ &_mtx_resolvers };
        _resolvers.push( WJP_WACK_Resolver_RTG{ info: info, noun: _wack_nouner.fetch_add( 1, WJP_MEM_ORD_RELEASE ) } );

        WJP_ScopedLock lock_send{ &_mtx_send };
        lock_resolvers.release();
        int status = _inter_mech->send( WJP_MDsc_v{ addr: ( void* )head, sz: sizeof( WJP_Head ) }, flags, _arg );

        WJP_ASSERT_OR( status == sizeof( WJP_Head ) ) {
            WJP_INTER_MECH_INFO_ERR( status, WJPErr_Send );
        }

        return status;
    }
};