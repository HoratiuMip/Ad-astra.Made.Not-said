#pragma once
/*===== Warp Joint Protocol - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Yes.
|
======*/
#include "wjp_internals.hpp"

#include <functional>


#if defined( WJP_ARCHITECTURE_BIG ) 
    const int32_t WJP_SIG     = 0x57'4a'50'00; 
    const int32_t WJP_SIG_MSK = 0xff'ff'ff'00;
#else
    const int32_t WJP_SIG     = 0x00'50'4a'57;
    const int32_t WJP_SIG_MSK = 0x00'ff'ff'ff;
    #if !defined( WJP_ARCHITECTURE_LITTLE )
        #warning "[ WJP ] Endianess of target architecture not specified. Defaulting to little endian."
    #endif
#endif


enum WJPOp_ : int8_t {
    WJPOp_Null   = 0x0,

    WJPOp_Ack    = 0x1,
    WJPOp_Nak    = 0x2,

    WJPOp_Ping   = 0xb,
    
    WJPOp_QSet   = 0xc,
    WJPOp_QGet   = 0xd,

    WJPOp_IBurst = -0xb,

    _WJPOp_FORCE_BYTE = 0x7f
};

struct WJP_HEAD {
    constexpr WJP_HEAD() { _dw0._sig_b0 = 0x57; _dw0._sig_b1 = 0x4a, _dw0._sig_b2 = 0x50; _dw0.op = WJPOp_Null; }

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
#define WJP_QSET_LAMBDA ( WJP_BUFFER args ) -> const char*
#define WJP_QGET_LAMBDA ( void* dest, WJP_BUFFER args ) -> const char*

#define WJP_QGSTBL_READ_WRITE 0
#define WJP_QGSTBL_READ_ONLY  1

/**
 * @brief Called by WJP when the endpoint requests a QSet.
 * @returns The reason why the QSet was NOT acknowledged, as a NULL-terminated string, or NULL if the QSet was acknowledged.
 * @param WJP_BUFFER: The QSet arguments.
 */
typedef   std::function< const char*( WJP_BUFFER ) >          WJP_QGSTBL_QSET_FUNC;
/**
 * @brief Called by WJP when the endpoint requests a QGet.
 * @returns The reason why the QGet was NOT acknowledged, as a NULL-terminated string, or NULL if the QGet was acknowledged.
 * @param void*: The beginning address of where to write the data corresponding to the QGet request. The length, in bytes, of the writable area is specified in the QGSTBL_ENTRY, by the host.
 * @param WJP_BUFFER: The QGet arguments.
 */
typedef   std::function< const char*( void*, WJP_BUFFER ) >   WJP_QGSTBL_QGET_FUNC;

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
#define WJP_IBRST_DIR_IN 0
#define WJP_IBRST_DIR_OUT 1
#define WJP_IBRST_DIR_INOUT 2

#define WJP_IBRST_UNPACKED 0
#define WJP_IBRST_PACKED 1

typedef   std::function< void*( void ) >   WJP_IBRST_OUT_FUNC;

#define WJP_IBRST_OUT_LAMBDA ( void ) -> void*

typedef   int   WJPIBrstStatus_;
#define WJPIBrstStatus_Disengaged 0
#define WJPIBrstStatus_Engaged -1

struct WJP_IBRSTBL_ENTRY {
    void*                mem           = nullptr;
    int32_t              sz            = 0;
    WJP_IBRST_OUT_FUNC   out_fnc       = nullptr;
    unsigned int         dir     : 2;
    bool                 packed;
    int                  _status       = WJPIBrstStatus_Disengaged;
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
#define WJP_SEND_LAMBDA ( WJP_CBUFFER src, int flags ) -> int
#define WJP_RECV_LAMBDA ( WJP_BUFFER dst, int flags ) -> int

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
typedef   std::function< int( WJP_CBUFFER, int ) >   WJP_SEND_FUNC;
/**
 * @brief Called by WJP when the protocol needs to receive data from the endpoint.
 * @warning This function must guarantee that all the requested bytes are received.
 * @returns The count of receive bytes, or a negative value in case of an error and zero in case of a connection reset.
 * @param void*: The beginning address of where to receive the data.
 * @param int32_t: The length, in bytes, of the data to be received.
 * @param int: Flags.
 */
typedef   std::function< int( WJP_BUFFER, int ) >    WJP_RECV_FUNC;

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

/*
|>  DEVICE ======
*/
#define WJP_NAKR_MAX_SIZE 32

#define _WJP_ASSERT( c, r ) if( !( c ) ) return ( r );
#define _WJP_ASSERT_INFO( c, e, r ) if( !( c ) ) { info->err = e; return ( r ); }
#define _WJP_ASSERT_CTX_INFO( c, e, r ) if( !( c ) ) { context->info->err = e; return ( r ); }

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
    WJPErr_Forced     = 9,

    /* The receive function wrap did not read the byte count requested by WJP. */
    WJPErr_CountRecv  = 10,

    /* The send function wrap did not write the byte count requested by WJP. */
    WJPErr_CountSend  = 11,

    /* The received packet has an invalid operation code. */
    WJPErr_InvalidOp  = 12
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
    "WJPErr_Forced",
    "WJPErr_CountRecv",
    "WJPErr_CountSend",
    "WJPErr_InvalidOp"
};

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
typedef   std::function< int16_t( void ) >   WJP_SEQ_ACQ_FUNC;

struct WJP_DEVICE {
/*
|>  FIELDS ======
*/
    WJP_SRWRAP                                _srwrap      = {};
    WJP_BUFFER                                _recv_buf    = {};

    WJP_SEQ_ACQ_FUNC                          _seq_acq     = nullptr;

    WJP_QGSTBL                                _qgstbl      = {};
    WJP_IBRSTBL                               _ibrstbl     = {};

    std::mutex                                _smtx        = {};

    _WJP_INTERLOCKED_QUEUE< _WJP_RESOLVER >   _resolvers   = {};

/*
|>  BINDINGS ======
*/
    void bind_srwrap( const WJP_SRWRAP& srwrap ) { _srwrap = srwrap; }
    void bind_recv_buf( const WJP_BUFFER& buf ) { _recv_buf = buf; }

    void bind_seq_acq( const WJP_SEQ_ACQ_FUNC& seq_acq ) { _seq_acq = seq_acq; }

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
                std::unique_lock< decltype( _smtx ) > lock{ _smtx };
                
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
                
                std::unique_lock< decltype( _smtx ) > lock{ _smtx }; 
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

    _WJP_forceinline int _resolve_ping( _WJP_RECV_CONTEXT* context ) {
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
        _WJP_ASSERT_CTX_INFO( !_resolvers.empty(), WJPErr_NoResolver, -1 );

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

    int resolve_recv( WJP_RESOLVE_RECV_INFO* info ) {
        _WJP_RECV_CONTEXT _context, *context = &_context;
        context->info = info;

        _WJP_RR_ASSERT_RECV(
            _srwrap.recv( WJP_BUFFER{ addr: &context->info->recv_head, sz: sizeof( WJP_HEAD ) }, 0 ), 
            sizeof( WJP_HEAD ), 
            WJPErr_Recv
        );

        _WJP_ASSERT_CTX_INFO( context->info->recv_head.is_signed(), WJPErr_NotSigned, -1 );
        _WJP_ASSERT_CTX_INFO( context->info->recv_head._dw3.sz >= 0, WJPErr_Size, -1 );

        switch( context->info->recv_head._dw0.op ) {
            case WJPOp_Ping: {
                _WJP_RR_ASSERT_AGENT( this->_resolve_ping( context ) );
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
        }

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
        _WJP_RESOLVER& resolver = _resolvers.emplace( _WJP_RESOLVER{
            seq:  _seq_acq(),
            dst:  dst,
            info: info
        } );

        int ret = _send<>( src.addr, src.sz, op, resolver.seq, 0, method );
        _WJP_WB_ASSERT__SEND( ret, WJPErr_Send );

        return ret;
    }

    void force_waiting_resolvers() {
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
        std::unique_lock< std::mutex > lock{ _smtx };
        return _srwrap.send( WJP_CBUFFER{ addr: str, sz: strlen( str ) }, 0 ) == strlen( str );
    }

    int resolve_ibrst( int entry_index, int flags ) {
        WJP_IBRSTBL_ENTRY* entry = &_ibrstbl.entries[ entry_index ];

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
|>  WRAPPERS ======
*/
    //_WJP_forceinline int qget( WJP_WAIT_BACK_INFO* info, WJP_BUFFER )

};

