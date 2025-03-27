#pragma once
/*===== Warp Joint Protocol - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Yes.
|
======*/
#include "wjp_internals.hpp"

#include <deque>
#include <functional>
#include <atomic>
#include <mutex>


#if defined( WJP_ARCHITECTURE_BIG ) 
    const int32_t WJP_SIG     = 0x42'41'52'00; 
    const int32_t WJP_SIG_MSK = 0xff'ff'ff'00;
#else
    const int32_t WJP_SIG     = 0x00'52'41'42;
    const int32_t WJP_SIG_MSK = 0x00'ff'ff'ff;
    #if !defined( WJP_ARCHITECTURE_LITTLE )
        #warning "[ WJP ] Endianess of target architecture not specified. Defaulting to little endian."
    #endif
#endif


enum WJPOp_ : int8_t {
    WJPOp_Null    = 0,

    WJPOp_Ack     = 1,
    WJPOp_Nak     = 2,

    WJPOp_Ping    = 11,
    WJPOp_QSet     = 12,
    WJPOp_QGet     = 13,

    WJPOp_IBurst   = -11,

    _WJPOp_FORCE_BYTE = 0x7f
};

struct WJP_HEAD {
    constexpr WJP_HEAD() { _dw0._sig_b0 = 0x42; _dw0._sig_b1 = 0x41, _dw0._sig_b2 = 0x52; _dw0.op = WJPOp_Null; }

    union {
        struct{ int8_t _sig_b0, _sig_b1, _sig_b2; int8_t op; } _dw0;
        int32_t                                                sig;
    };
    struct{ int16_t _reserved; int16_t seq = 0; }              _dw1;
    struct{ int32_t _reserved; }                               _dw2;
    struct { int32_t sz = 0;  }                                _dw3;

    _WJP_forceinline bool is_signed( void ) { return ( sig & WJP_SIG_MSK ) == WJP_SIG; }
};
static_assert( sizeof( WJP_HEAD ) == 4*sizeof( int32_t ) );


/*
|>  QUICK GET/SET ======
*/
#define WJP_QGSTBL_READ_WRITE 0
#define WJP_QGSTBL_READ_ONLY  1

/**
 * @brief Called by WJP when the endpoint requests a QSet.
 * @returns The reason why the QSet was NOT acknowledged, as a NULL-terminated string, or NULL if the QSet was acknowledged.
 * @param void*: The beginning address of the QSet arguments.
 * @param int32_t: The length, in bytes, of the QSet arguments. 
 */
typedef   std::function< const char*( void*, int32_t ) >   WJP_QGSTBL_QSET_FUNC;
/**
 * @brief Called by WJP when the endpoint requests a QGet.
 * @returns The reason why the QGet was NOT acknowledged, as a NULL-terminated string, or NULL if the QGet was acknowledged.
 * @param void*: The beginning address of where to write the data corresponding to the QGet request. The length, in bytes, of the writable area is specified in the QGSTBL_ENTRY.
 */
typedef   std::function< const char*( void* ) >            WJP_QGSTBL_QGET_FUNC;

struct WJP_QGSTBL_ENTRY {
    const char*            str_id;
    int32_t                sz;
    const bool             read_only;    
    WJP_QGSTBL_QSET_FUNC   qset_func;
    WJP_QGSTBL_QGET_FUNC   qget_func;     
    void*                  src;    
};

struct WJP_QGSTBL {
    WJP_QGSTBL_ENTRY*   entries   = nullptr;
    int                 count     = 0;

    WJP_QGSTBL_ENTRY* query( const char* str_id ) {
        for( int idx = 0; idx < count; ++idx ) {
            if( strcmp( entries[ idx ].str_id, str_id ) == 0 ) return &entries[ idx ];
        }
        return nullptr;
    }
};

/*
|>  INDEXED BURST ======
*/
struct WJP_IBRSTBL_ENTRY {
    void*     dst;
    int32_t   sz;
};

struct WJP_IBRSTBL {
    WJP_IBRSTBL_ENTRY*   entries   = nullptr;
    int                  count     = 0;

    WJP_IBRSTBL_ENTRY* operator [] ( int idx ) {
        if( idx >= count || idx < 0 ) return nullptr;
        return &entries[ idx ];
    }
};

/*
|>  SEND/RECV ======
*/
#define WJP_SEND_LAMBDA ( const void* src, int32_t sz, int flags ) -> int
#define WJP_RECV_LAMBDA ( void* dst, int32_t sz, int flags ) -> int

#define WJP_NO_HEAD  0
#define WJP_HAS_HEAD 1

/**
 * @brief Called by WJP when the protocol needs to send data to the endpoint.
 * @warning This function must guarantee that all the requested bytes are sent.
 * @returns The count of sent bytes, or a negative value in case of an error and zero in case of a connection reset.
 * @param const_void*: The beginning address of the data to be sent.
 * @param int32_t: The length, in bytes, of the data to be sent.
 * @param int: Flags.
 */
typedef   std::function< int( const void*, int32_t, int ) >   WJP_SEND_FUNC;
/**
 * @brief Called by WJP when the protocol needs to receive data from the endpoint.
 * @warning This function must guarantee that all the requested bytes are received.
 * @returns The count of receive bytes, or a negative value in case of an error and zero in case of a connection reset.
 * @param void*: The beginning address of where to receive the data.
 * @param int32_t: The length, in bytes, of the data to be received.
 * @param int: Flags.
 */
typedef   std::function< int( void*, int32_t, int ) >         WJP_RECV_FUNC;

struct WJP_SRWRAP {
    WJP_SEND_FUNC   send    = nullptr;
    WJP_RECV_FUNC   recv    = nullptr;
};

enum WJPSendMethod_ {
    /* Make a call to send for each source. */
    WJPSendMethod_Direct,

    /* Copy on the stack from all the sources and then make one call to send. */
    WJPSendMethod_Stack,

    /* Let WJP decide, based on the underlying system and the count of bytes to send. */
    WJPSendMethod_Auto
};

struct WJP_SEND_N_DESC {
    const void*   src;
    int32_t       sz;
};

/*
|>  DEVICE ======
*/
#define WJP_NAKR_MAX_SIZE 32

#define _WJP_ASSERT( c, r ) if( !( c ) ) return ( r );
#define _WJP_ASSERT_INFO( c, e, r ) if( !( c ) ) { info->err = e; return ( r ); }

enum WJPErr_ : int {
    /* No error. */
    WJPErr_None       = 0,

    /* The received header is missing the WJP signature. */
    WJPErr_NotSigned  = 1,

    /* Error during reception. */
    WJPErr_Recv       = 2,

    /* Error during transmission. */
    WJPErr_Send       = 3,

    /* The received packet has no resolver waiting for it. */
    WJPErr_NoResolver = 4,

    /* The received packet is out of sequence. */
    WJPErr_Sequence   = 5,

    /* The received packet reports a wrong size. */
    WJPErr_Size       = 6,

    /* Either the host or the endpoint ended the connection. */
    WJPErr_ConnReset  = 7,

    /* The received packet has an unidentifiable id. */
    WJPErr_NoEntry    = 8,

    /* The host forced the wake of the waiting thread. */
    WJPErr_Forced     = 9
};
const char* WJP_err_strs[] = {
    "WJPErr_None",
    "WJPErr_NotSigned",
    "WJPErr_Recv",
    "WJPErr_Send",
    "WJPErr_NoResolver",
    "WJPErr_Sequence",
    "WJPErr_Size",
    "WJPErr_ConnReset",
    "WJPErr_NoEntry",
    "WJPErr_Forced"
};

struct WJP_RESOLVE_RECV_INFO {
    WJP_HEAD      recv_head    = {};
    int32_t       sent_count   = 0;
    WJPErr_       err          = WJPErr_None;
    const char*   nakr         = nullptr;
};

struct WJP_WAIT_BACK_INFO {
    std::atomic_bool   resolved                        = { false };
    int32_t            recv_count                      = 0;
    WJPErr_            err                             = WJPErr_None;
    char               nakr[ WJP_NAKR_MAX_SIZE + 1 ]   = { '\0' };

    _WJP_forceinline bool ackd( void ) { return nakr[ 0 ] == '\0'; }
};

struct _WJP_RESOLVER {
    int16_t               _seq    = 0;
    void*                 _dst    = nullptr;
    int32_t               _sz     = 0;
    WJP_WAIT_BACK_INFO*   _info   = nullptr;
};

/**
 * @brief Called by WJP when a new sequence for a head is required. Usually an interlocked increment does the job.
 * @returns The sequence.
 */
typedef   std::function< int16_t( void ) >   WJP_SEQ_ACQ_FUNC;

struct WJP_DEVICE {
    WJP_SRWRAP                    _srwrap      = {};
    WJP_QGSTBL                    _qgstbl      = {};
    WJP_IBRSTBL                   _ibrstbl     = {};
    std::mutex                    _smtx        = {};
    WJP_SEQ_ACQ_FUNC              _seq_acq     = nullptr;
    std::deque< _WJP_RESOLVER >   _resolvers   = {};
    std::mutex                    _resmtx      = {};


    void bind_srwrap( const WJP_SRWRAP& srwrap ) { _srwrap = srwrap; }
    void bind_qgstbl( const WJP_QGSTBL& qgstbl ) { _qgstbl = qgstbl; }
    void bind_ibrstbl( const WJP_IBRSTBL& ibrstbl ) { _ibrstbl = ibrstbl; }
    void bind_seq_acq( const WJP_SEQ_ACQ_FUNC& seq_acq ) { _seq_acq = seq_acq; }


    int32_t _sizeof_send_n_descs( WJP_SEND_N_DESC* descs, int count ) {
        int sz = 0;
        for( int idx = 0; idx < count; ++idx ) sz += descs[ idx ].sz;
        return sz;
    }

    template< bool _HAS_HEAD = WJP_NO_HEAD >
    int _send_n( WJP_SEND_N_DESC* descs, int count, WJPOp_ op, int16_t seq, int flags, WJPSendMethod_ method ) {
        int ret = 0;

        switch( method ) {
        l_direct: case WJPSendMethod_Direct: {
                std::unique_lock< decltype( _smtx ) > lock{ _smtx };
                
                if constexpr( WJP_NO_HEAD == _HAS_HEAD ) {
                    WJP_HEAD head;
                    head._dw0.op  = op;
                    head._dw1.seq = seq;
                    head._dw3.sz  = _sizeof_send_n_descs( descs, count );

                    int crt_ret = _srwrap.send( &head, sizeof( head ), flags );
                    _WJP_ASSERT( crt_ret == sizeof( head ), crt_ret <= 0 ? crt_ret : -1 );
                    ret += crt_ret;
                }

                for( int idx = 0; idx < count; ++idx ) {
                    int crt_ret = _srwrap.send( descs[ idx ].src, descs[ idx ].sz, flags );
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
                    memcpy( buffer + tot_sz, descs[ idx ].src, descs[ idx ].sz );
                    tot_sz += descs[ idx ].sz;
                }
                
                std::unique_lock< decltype( _smtx ) > lock{ _smtx }; 
                ret = _srwrap.send( buffer, tot_sz, flags );
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
        WJP_SEND_N_DESC desc{ src: src_, sz: sz_ };
        if( src_ != nullptr )
            return _send_n< _HAS_HEAD >( &desc, 1, op, seq, flags, method );
        return _send_n< _HAS_HEAD >( nullptr, 0, op, seq, flags, method );
    }

    
#define _WJP_RR_ASSERT__SEND( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? WJPErr_ConnReset : e; return _sret; } info->sent_count += _sret; }
#define _WJP_RR_NAK_IF( c, r ) if( c ) { info->nakr = "WJP_NAKR_" r; goto l_gs_nak; }
#define _WJP_RR_ASSERT_RECV( r, t, e ) { int _rret = ( r ); if( _rret <= 0 ) { info->err = ( _rret == 0 ) ? WJPErr_ConnReset : e; return _rret; } if( _rret != ( t ) ) return -1; }
    int resolve_recv( WJP_RESOLVE_RECV_INFO* info ) {
        _WJP_RR_ASSERT_RECV(
            _srwrap.recv( &info->recv_head, sizeof( WJP_HEAD ), 0 ), sizeof( WJP_HEAD ), WJPErr_Recv
        );

        _WJP_ASSERT_INFO( info->recv_head.is_signed(), WJPErr_NotSigned, -1 );

        switch( info->recv_head._dw0.op ) {
            case WJPOp_Ping: {
                _WJP_RR_ASSERT__SEND( 
                    _send<>( nullptr, 0, WJPOp_Ack, info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ), 
                    WJPErr_Send 
                );
            break; }

            case WJPOp_QGet: [[fallthrough]];
            case WJPOp_QSet: {
                _WJP_RR_NAK_IF( info->recv_head._dw3.sz <= 0, "BAD_SIZE" );
                char buffer[ info->recv_head._dw3.sz ];
                _WJP_RR_ASSERT_RECV(
                    _srwrap.recv( buffer, info->recv_head._dw3.sz, 0 ), info->recv_head._dw3.sz, WJPErr_Recv
                );

                char* delim = buffer - 1; while( *++delim != '\0' ) {
                    _WJP_RR_NAK_IF( delim - buffer >= info->recv_head._dw3.sz - 1, "BAD_FORMAT" );
                }

                WJP_QGSTBL_ENTRY* entry = _qgstbl.query( buffer );
                _WJP_RR_NAK_IF( entry == nullptr, "NO_ENTRY" );

                if( info->recv_head._dw0.op == WJPOp_QSet ) goto l_set;
            l_get:
                if( entry->src != nullptr ) {
                    _WJP_RR_ASSERT__SEND( 
                        _send<>( entry->src, entry->sz, WJPOp_Ack, info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
                        WJPErr_Send
                    );
                } else {
                    _WJP_RR_NAK_IF( true, "NO_PROCEDRE" );
                }
            break;
            l_set:
                _WJP_RR_NAK_IF( entry->read_only, "READ_ONLY" );

                int32_t sz = info->recv_head._dw3.sz - ( delim - buffer + 1 );
                _WJP_RR_NAK_IF( entry->sz != sz, "SIZE_MISMATCH" );

                if( entry->qset_func != nullptr ) {
                    if( const char* nakr = entry->qset_func( delim + 1, sz ); nakr != nullptr ) {
                        info->nakr = nakr; goto l_gs_nak; 
                    }
                } else if( entry->src != nullptr ) {
                    memcpy( entry->src, delim + 1, sz );
                } else {
                    _WJP_RR_NAK_IF( true, "NO_PROCEDRE" );
                }
                _WJP_RR_ASSERT__SEND( 
                    _send<>( nullptr, 0, WJPOp_Ack, info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
                    WJPErr_Send
                );
            break; }

            l_gs_nak: {
                _WJP_RR_ASSERT__SEND(
                    _send( info->nakr, info->nakr ? strlen( info->nakr ) + 1 : 0, WJPOp_Nak, info->recv_head._dw1.seq, 0, WJPSendMethod_Direct ),
                    WJPErr_Send 
                );

            break; }

            case WJPOp_Nak: [[fallthrough]];
            case WJPOp_Ack: {
                _WJP_ASSERT_INFO( !_resolvers.empty(), WJPErr_NoResolver, -1 );

                _WJP_RESOLVER& resolver = _resolvers.front();

                _WJP_ASSERT_INFO( info->recv_head._dw1.seq == resolver._seq, WJPErr_Sequence, -1 );

                WJP_WAIT_BACK_INFO* wb_info = resolver._info;

                if( info->recv_head._dw0.op == WJPOp_Nak ) {
                    //_WJP_ASSERT_INFO( info->recv_head._dw3.sz <= WJP_NAKR_MAX_SIZE, WJP_ERR_NAKR_SIZE, -1 );
                    _WJP_RR_ASSERT_RECV(
                        _srwrap.recv( wb_info->nakr, info->recv_head._dw3.sz, 0 ), info->recv_head._dw3.sz, WJPErr_Recv
                    );
                    goto l_resolver_sig;
                }

                if( resolver._dst == nullptr || info->recv_head._dw3.sz <= 0 ) goto l_resolver_sig;

                _WJP_ASSERT_INFO( resolver._sz >= info->recv_head._dw3.sz, WJPErr_Size, -1 );
                _WJP_RR_ASSERT_RECV(
                    _srwrap.recv( resolver._dst, info->recv_head._dw3.sz, 0 ), info->recv_head._dw3.sz, WJPErr_Recv
                );

                wb_info->recv_count += info->recv_head._dw3.sz;

            l_resolver_sig:
                wb_info->recv_count += sizeof( WJP_HEAD );
                
                std::unique_lock< std::mutex > lock{ _resmtx };
                _resolvers.pop_front();
                lock.unlock();
                
                wb_info->resolved.store( true, std::memory_order_release );
            #if defined( WJP_NOTIFIABLE_ATOMICS )
                wb_info->resolved.notify_one();
            #endif
            break; }

            case WJPOp_IBurst: {
                WJP_IBRSTBL_ENTRY* entry = _ibrstbl[ info->recv_head._dw1.seq ];
                _WJP_ASSERT_INFO( entry != nullptr, WJPErr_NoEntry, -1 );
                _WJP_ASSERT_INFO( info->recv_head._dw3.sz <= entry->sz, WJPErr_Size, -1 );

                _WJP_RR_ASSERT_RECV(
                    _srwrap.recv( entry->dst, info->recv_head._dw3.sz, 0 ), info->recv_head._dw3.sz, WJPErr_Recv
                );
            break; }

        }
        
        return sizeof( WJP_HEAD ) + info->recv_head._dw3.sz;
    }

#define _WJP_WB_ASSERT__SEND( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? WJPErr_ConnReset : e; return _sret; } }
    int wait_back( 
        WJP_WAIT_BACK_INFO* info, 
        WJPOp_ op, 
        const void* src, int32_t src_sz, 
        void* dst, int32_t dst_sz,
        WJPSendMethod_ method
    ) {
        std::unique_lock< std::mutex > lock{ _resmtx };
        _WJP_RESOLVER& resolver = _resolvers.emplace_back( _WJP_RESOLVER{
            _seq: _seq_acq(),
            _dst: dst,
            _sz:  dst_sz,
            _info: info
        } );
        lock.unlock();

        int ret = _send<>( src, src_sz, op, resolver._seq, 0, method );
        _WJP_WB_ASSERT__SEND( ret, WJPErr_Send );

        return ret;
    }

    void force_waiting_resolvers() {
        std::unique_lock< std::mutex > lock{ _resmtx };
        for( auto& resolver : _resolvers ) {
            strcpy( resolver._info->nakr, WJP_err_strs[ WJPErr_Forced ] );
            resolver._info->err = WJPErr_Forced;
            resolver._info->resolved.store( true, std::memory_order_release );
        #if defined( WJP_NOTIFIABLE_ATOMICS )
            resolver._info->resolved.notify_one();
        #endif
        }
    }

    bool breach( void ) {
        const char str[] = "WJP_INTENTIONAL_BREACH";
        std::unique_lock< std::mutex > lock{ _smtx };
        return _srwrap.send( str, strlen( str ), 0 ) == strlen( str );
    }

    int trust_burst( const void* src, int32_t sz, int flags ) {
        return _srwrap.send( src, sz, flags );
    }

};

