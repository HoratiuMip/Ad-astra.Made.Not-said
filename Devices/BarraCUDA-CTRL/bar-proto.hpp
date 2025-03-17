#pragma once
/*===== BAR protocol - Vatca "Mipsan" Tudor-Horatiu
|
> Header that deals with the BAR protocol, offering wrappers over transmission procedures.
|
======*/
#include <deque>
#include <functional>
#include <atomic>
#include <mutex>



#if defined( BAR_PROTO_ARCHITECTURE_BIG ) 
    const int32_t BAR_PROTO_SIG     = 0x42'41'52'00; 
    const int32_t BAR_PROTO_SIG_MSK = 0xff'ff'ff'00;
#elif defined( BAR_PROTO_ARCHITECTURE_LITTLE )
    const int32_t BAR_PROTO_SIG     = 0x00'52'41'42;
    const int32_t BAR_PROTO_SIG_MSK = 0x00'ff'ff'ff;
#else
    #error "BAR protocol: Endianess of target architecture not specified."
#endif

enum BAR_PROTO_OP : int8_t {
    BAR_PROTO_OP_NULL    = 0,

    BAR_PROTO_OP_ACK     = 1,
    BAR_PROTO_OP_NAK     = 2,

    BAR_PROTO_OP_PING    = 11,
    BAR_PROTO_OP_SET     = 12,
    BAR_PROTO_OP_GET     = 13,

    BAR_PROTO_OP_BURST   = -11,

    _BAR_PROTO_OP_FORCE_BYTE = 0x7f
};

struct bar_proto_head_t {
    bar_proto_head_t() { _dw0._sig_b0 = 0x42; _dw0._sig_b1 = 0x41, _dw0._sig_b2 = 0x52; _dw0.op = BAR_PROTO_OP_NULL; }

    union {
        struct{ int8_t _sig_b0, _sig_b1, _sig_b2; int8_t op; } _dw0;
        int32_t                                                sig;
    };
    struct{ int16_t _reserved; int16_t seq = 0; }              _dw1;
    struct { int32_t sz = 0;  }                                _dw2;

    bool is_signed( void ) { return ( sig & BAR_PROTO_SIG_MSK ) == BAR_PROTO_SIG; }
};
static_assert( sizeof( bar_proto_head_t ) == 3*sizeof( int32_t ) );

template< int16_t BUF_SZ >
struct bar_cache_t {
    std::mutex                mtx;
    char                      _buffer[ BUF_SZ ];
    bar_proto_head_t* const   head                = ( bar_proto_head_t* )_buffer;
    void* const               data                = _buffer + sizeof( *head );
    
    bar_cache_t() { *head = bar_proto_head_t{}; }
};



typedef   std::function< const char*( void*, int32_t ) >   bar_proto_gstbl_set_func_t;
#define BAR_PROTO_GSTBL_READ_WRITE 0
#define BAR_PROTO_GSTBL_READ_ONLY  1
struct BAR_PROTO_GSTBL_ENTRY {
  const char* const            str_id;
  void* const                  src;
  const int32_t                sz;
  const bool                   read_only;    
  bar_proto_gstbl_set_func_t   set;         
};
struct BAR_PROTO_GSTBL {
  BAR_PROTO_GSTBL_ENTRY*   entries   = nullptr;
  int                      size      = 0;

  BAR_PROTO_GSTBL_ENTRY* search( const char* str_id ) {
    for( int idx = 0; idx < size; ++idx ) {
      if( strcmp( entries[ idx ].str_id, str_id ) == 0 ) return &entries[ idx ];
    }
    return nullptr;
  }

};
struct BAR_PROTO_BRSTBL_ENTRY {
    void*     dst;
    int32_t   sz;
};
struct BAR_PROTO_BRSTBL {
    BAR_PROTO_BRSTBL_ENTRY*   entries   = nullptr;
    int                       size      = 0;

    BAR_PROTO_BRSTBL_ENTRY* operator [] ( int idx ) {
        if( idx > size || idx <= 0 ) return nullptr;
        return &entries[ idx - 1 ];
    }
};

typedef   std::function< int( const void*, int32_t, int ) >   bar_proto_send_func_t;
typedef   std::function< int( void*, int32_t, int ) >         bar_proto_recv_func_t;
#define BAR_PROTO_SEND_LAMBDA ( const void* src, int32_t sz, int flags ) -> int
#define BAR_PROTO_RECV_LAMBDA ( void* dst, int32_t sz, int flags ) -> int
struct BAR_PROTO_SRWRAP {
    bar_proto_send_func_t   send    = nullptr;
    bar_proto_recv_func_t   recv    = nullptr;
};

#define BAR_PROTO_NO_HEAD  0
#define BAR_PROTO_HAS_HEAD 1
enum BAR_PROTO_SEND_METHOD {
    BAR_PROTO_SEND_METHOD_DIRECT,
    BAR_PROTO_SEND_METHOD_STACK,
    BAR_PROTO_SEND_METHOD_AUTO
};
enum BAR_PROTO_ERR : int {
    BAR_PROTO_ERR_NONE        = 0,
    BAR_PROTO_ERR_NOT_SIGNED  = 1,
    BAR_PROTO_ERR_RECV        = 2,
    BAR_PROTO_ERR_SEND        = 3,
    BAR_PROTO_ERR_NO_RESOLVER = 4,
    BAR_PROTO_ERR_SEQUENCE    = 5,
    BAR_PROTO_ERR_SIZE        = 6,
    BAR_PROTO_ERR_NAKR_SIZE   = 7,
    BAR_PROTO_ERR_CONN_RESET  = 8,
    BAR_PROTO_ERR_NO_ENTRY    = 9,
    BAR_PROTO_ERR_FORCED      = 10
};
const char* BAR_PROTO_ERR_STR[] = {
    "BAR_PROTO_ERR_NONE",
    "BAR_PROTO_ERR_NOT_SIGNED",
    "BAR_PROTO_ERR_RECV",
    "BAR_PROTO_ERR_SEND",
    "BAR_PROTO_ERR_NO_RESOLVER",
    "BAR_PROTO_ERR_SEQUENCE",
    "BAR_PROTO_ERR_SIZE",
    "BAR_PROTO_ERR_NAKR_SIZE",
    "BAR_PROTO_ERR_CONN_RESET",
    "BAR_PROTO_ERR_NO_ENTRY",
    "BAR_PROTO_ERR_FORCED"
};
struct BAR_PROTO_RESOLVE_RECV_INFO {
    bar_proto_head_t       recv_head   = {};
    int32_t                sent        = 0;
    BAR_PROTO_ERR          err         = BAR_PROTO_ERR_NONE;
    const char*            nakr        = nullptr;
};
#define BAR_PROTO_NAKR_MAX_SIZE 32
struct BAR_PROTO_WAIT_BACK_INFO {
    std::atomic_bool       sig                                   = { false };
    int32_t                recvd                                 = 0;
    bool                   ackd                                  = false;
    BAR_PROTO_ERR          err                                   = BAR_PROTO_ERR_NONE;
    char                   nakr[ BAR_PROTO_NAKR_MAX_SIZE + 1 ]   = { '\0' };
};
struct BAR_PROTO_SEND_N_DESC {
    const void*   src;
    int32_t       sz;
};
struct _BAR_PROTO_RESOLVER {
    int16_t                     _seq    = 0;
    void*                       _dst    = nullptr;
    int32_t                     _sz     = 0;
    BAR_PROTO_WAIT_BACK_INFO*   _info   = nullptr;
};
typedef   std::function< int16_t( void ) >   bar_proto_seq_acq_func_t;
#define _BAR_PROTO_ASSERT( c, r ) if( !( c ) ) return ( r );
#define _BAR_PROTO_INFO_ASSERT( c, e ) if( !( c ) ) { info->err = e; return -1; }
struct BAR_PROTO_STREAM {
    BAR_PROTO_GSTBL                     _gstbl       = {};
    BAR_PROTO_BRSTBL                    _brstbl      = {};
    BAR_PROTO_SRWRAP                    _srwrap      = {};
    std::mutex                          _smtx        = {};
    bar_proto_seq_acq_func_t            _seq_acq     = nullptr;
    std::deque< _BAR_PROTO_RESOLVER >   _resolvers   = {};
    std::mutex                          _resmtx      = {};


    void bind_gstbl( const BAR_PROTO_GSTBL& gstbl ) { _gstbl = gstbl; }
    void bind_brstbl( const BAR_PROTO_BRSTBL& brstbl ) { _brstbl = brstbl; }
    void bind_srwrap( const BAR_PROTO_SRWRAP& srwrap ) { _srwrap = srwrap; }
    void bind_seq_acq( const bar_proto_seq_acq_func_t& seq_acq ) { _seq_acq = seq_acq; }


    int32_t _sizeof_send_n_descs( BAR_PROTO_SEND_N_DESC* descs, int count ) {
        int sz = 0;
        for( int idx = 0; idx < count; ++idx ) sz += descs[ idx ].sz;
        return sz;
    }

    template< bool _HAS_HEAD = BAR_PROTO_NO_HEAD >
    int _send_n( BAR_PROTO_SEND_N_DESC* descs, int count, BAR_PROTO_OP op, int16_t seq, int flags, BAR_PROTO_SEND_METHOD method ) {
        int ret = 0;

        switch( method ) {
        l_direct: case BAR_PROTO_SEND_METHOD_DIRECT: {
                std::unique_lock< decltype( _smtx ) > lock{ _smtx };
                
                if constexpr( BAR_PROTO_NO_HEAD == _HAS_HEAD ) {
                    bar_proto_head_t head;
                    head._dw0.op  = op;
                    head._dw1.seq = seq;
                    head._dw2.sz  = _sizeof_send_n_descs( descs, count );

                    int crt_ret = _srwrap.send( &head, sizeof( head ), flags );
                    _BAR_PROTO_ASSERT( crt_ret == sizeof( head ), crt_ret <= 0 ? crt_ret : -1 );
                    ret += crt_ret;
                }

                for( int idx = 0; idx < count; ++idx ) {
                    int crt_ret = _srwrap.send( descs[ idx ].src, descs[ idx ].sz, flags );
                    _BAR_PROTO_ASSERT( crt_ret == descs[ idx ].sz, crt_ret <= 0 ? crt_ret : -1 );
                    ret += crt_ret;
                }
            break; }

        l_stack: case BAR_PROTO_SEND_METHOD_STACK: {
                int tot_sz = _sizeof_send_n_descs( descs, count ) + _HAS_HEAD ? 0 : sizeof( bar_proto_head_t );

                char buffer[ tot_sz ];
                
                if constexpr( BAR_PROTO_NO_HEAD == _HAS_HEAD ) {
                    ( ( bar_proto_head_t* )buffer )->_dw0.op  = op;
                    ( ( bar_proto_head_t* )buffer )->_dw1.seq = seq;
                    ( ( bar_proto_head_t* )buffer )->_dw2.sz  = tot_sz - sizeof( bar_proto_head_t );
                    tot_sz = sizeof( bar_proto_head_t );
                } else {
                    tot_sz = 0;
                }

                for( int idx = 0; idx < count; ++idx ) {
                    memcpy( buffer + tot_sz, descs[ idx ].src, descs[ idx ].sz );
                    tot_sz += descs[ idx ].sz;
                }
                
                std::unique_lock< decltype( _smtx ) > lock{ _smtx }; 
                ret = _srwrap.send( buffer, tot_sz, flags );
                _BAR_PROTO_ASSERT( ret == tot_sz, ret <= 0 ? ret : -1 );
            break; }

            case BAR_PROTO_SEND_METHOD_AUTO: {
                if( descs == nullptr )
                    goto l_direct;
                else if( sizeof( bar_proto_head_t ) + _sizeof_send_n_descs( descs, count ) <= 64  )
                    goto l_stack;
                else 
                    goto l_direct;
            break; }
        }

        return ret;
    } 
    
    template< bool _HAS_HEAD = BAR_PROTO_NO_HEAD >
    int _send( const void* src_, int32_t sz_, BAR_PROTO_OP op, int16_t seq, int flags, BAR_PROTO_SEND_METHOD method ) {
        BAR_PROTO_SEND_N_DESC desc{ src: src_, sz: sz_ };
        if( src_ != nullptr )
            return _send_n< _HAS_HEAD >( &desc, 1, op, seq, flags, method );
        return _send_n< _HAS_HEAD >( nullptr, 0, op, seq, flags, method );
    }

    
#define _BAR_PROTO_RR__SEND_ASSERT( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? BAR_PROTO_ERR_CONN_RESET : e; return _sret; } info->sent += _sret; }
#define _BAR_PROTO_RR_NAK_IF( c, r ) if( c ) { info->nakr = "BAR_PROTO_NAKR_" r; goto l_gs_nak; }
#define _BAR_PROTO_RR_RECV_ASSERT( r, t, e ) { int _rret = ( r ); if( _rret <= 0 ) { info->err = ( _rret == 0 ) ? BAR_PROTO_ERR_CONN_RESET : e; return _rret; } if( _rret != ( t ) ) return -1; }
    int resolve_recv( BAR_PROTO_RESOLVE_RECV_INFO* info ) {
        _BAR_PROTO_RR_RECV_ASSERT(
            _srwrap.recv( &info->recv_head, sizeof( bar_proto_head_t ), 0 ), sizeof( bar_proto_head_t ), BAR_PROTO_ERR_RECV
        );

        _BAR_PROTO_INFO_ASSERT( info->recv_head.is_signed(), BAR_PROTO_ERR_NOT_SIGNED );

        switch( info->recv_head._dw0.op ) {
            case BAR_PROTO_OP_PING: {
                _BAR_PROTO_RR__SEND_ASSERT( 
                    _send<>( nullptr, 0, BAR_PROTO_OP_ACK, info->recv_head._dw1.seq, 0, BAR_PROTO_SEND_METHOD_DIRECT ), 
                    BAR_PROTO_ERR_SEND 
                );
            break; }

            case BAR_PROTO_OP_GET: [[fallthrough]];
            case BAR_PROTO_OP_SET: {
                _BAR_PROTO_RR_NAK_IF( info->recv_head._dw2.sz <= 0, "BAD_SIZE" );
                char buffer[ info->recv_head._dw2.sz ];
                _BAR_PROTO_RR_RECV_ASSERT(
                    _srwrap.recv( buffer, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_ERR_RECV
                );

                char* delim = buffer - 1; while( *++delim != '\0' ) {
                    _BAR_PROTO_RR_NAK_IF( delim - buffer >= info->recv_head._dw2.sz - 1, "BAD_FORMAT" );
                }

                BAR_PROTO_GSTBL_ENTRY* entry = _gstbl.search( buffer );
                _BAR_PROTO_RR_NAK_IF( entry == nullptr, "NO_ENTRY" );

                if( info->recv_head._dw0.op == BAR_PROTO_OP_SET ) goto l_set;
            l_get:
                if( entry->src != nullptr ) {
                    _BAR_PROTO_RR__SEND_ASSERT( 
                        _send<>( entry->src, entry->sz, BAR_PROTO_OP_ACK, info->recv_head._dw1.seq, 0, BAR_PROTO_SEND_METHOD_DIRECT ),
                        BAR_PROTO_ERR_SEND
                    );
                } else {
                    _BAR_PROTO_RR_NAK_IF( true, "NO_PROCEDRE" );
                }
            break;
            l_set:
                _BAR_PROTO_RR_NAK_IF( entry->read_only, "READ_ONLY" );

                int32_t sz = info->recv_head._dw2.sz - ( delim - buffer + 1 );
                _BAR_PROTO_RR_NAK_IF( entry->sz != sz, "SIZE_MISMATCH" );

                if( entry->set != nullptr ) {
                    if( const char* nakr = entry->set( delim + 1, sz ); nakr != nullptr ) {
                        info->nakr = nakr; goto l_gs_nak; 
                    }
                } else if( entry->src != nullptr ) {
                    memcpy( entry->src, delim + 1, sz );
                } else {
                    _BAR_PROTO_RR_NAK_IF( true, "NO_PROCEDRE" );
                }
                _BAR_PROTO_RR__SEND_ASSERT( 
                    _send<>( nullptr, 0, BAR_PROTO_OP_ACK, info->recv_head._dw1.seq, 0, BAR_PROTO_SEND_METHOD_DIRECT ),
                    BAR_PROTO_ERR_SEND
                );
            break; }

            l_gs_nak: {
                _BAR_PROTO_RR__SEND_ASSERT(
                    _send( info->nakr, info->nakr ? strlen( info->nakr ) + 1 : 0, BAR_PROTO_OP_NAK, info->recv_head._dw1.seq, 0, BAR_PROTO_SEND_METHOD_DIRECT ),
                    BAR_PROTO_ERR_SEND 
                );

            break; }

            case BAR_PROTO_OP_NAK: [[fallthrough]];
            case BAR_PROTO_OP_ACK: {
                _BAR_PROTO_INFO_ASSERT( !_resolvers.empty(), BAR_PROTO_ERR_NO_RESOLVER );

                _BAR_PROTO_RESOLVER& resolver = _resolvers.front();

                _BAR_PROTO_INFO_ASSERT( info->recv_head._dw1.seq == resolver._seq, BAR_PROTO_ERR_SEQUENCE );

                BAR_PROTO_WAIT_BACK_INFO* wb_info = resolver._info;

                if( info->recv_head._dw0.op == BAR_PROTO_OP_NAK ) {
                    _BAR_PROTO_INFO_ASSERT( info->recv_head._dw2.sz <= BAR_PROTO_NAKR_MAX_SIZE, BAR_PROTO_ERR_NAKR_SIZE );
                    _BAR_PROTO_RR_RECV_ASSERT(
                        _srwrap.recv( wb_info->nakr, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_ERR_RECV
                    );
                    goto l_resolver_sig;
                }

                if( resolver._dst == nullptr || info->recv_head._dw2.sz <= 0 ) goto l_resolver_sig;

                _BAR_PROTO_INFO_ASSERT( resolver._sz >= info->recv_head._dw2.sz, BAR_PROTO_ERR_SIZE );
                _BAR_PROTO_RR_RECV_ASSERT(
                    _srwrap.recv( resolver._dst, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_ERR_RECV
                );

                wb_info->recvd += info->recv_head._dw2.sz;

            l_resolver_sig:
                wb_info->recvd += sizeof( bar_proto_head_t );
                
                std::unique_lock< std::mutex > lock{ _resmtx };
                _resolvers.pop_front();
                lock.unlock();
                
                wb_info->ackd = info->recv_head._dw0.op == BAR_PROTO_OP_ACK;
                wb_info->sig.store( true, std::memory_order_release );
            #if defined( BAR_PROTO_NOTIFIABLE_ATOMICS )
                wb_info->sig.notify_one();
            #endif
            break; }

            case BAR_PROTO_OP_BURST: {
                BAR_PROTO_BRSTBL_ENTRY* entry = _brstbl[ info->recv_head._dw1.seq ];
                _BAR_PROTO_INFO_ASSERT( entry != nullptr, BAR_PROTO_ERR_NO_ENTRY );
                _BAR_PROTO_INFO_ASSERT( info->recv_head._dw2.sz <= entry->sz, BAR_PROTO_ERR_SIZE );

                _BAR_PROTO_RR_RECV_ASSERT(
                    _srwrap.recv( entry->dst, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_ERR_RECV
                );
            break; }

        }
        
        return sizeof( bar_proto_head_t ) + info->recv_head._dw2.sz;
    }

#define _BAR_PROTO_WB__SEND_ASSERT( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? BAR_PROTO_ERR_CONN_RESET : e; return _sret; } }
    int wait_back( 
        BAR_PROTO_WAIT_BACK_INFO* info, 
        BAR_PROTO_OP op, 
        const void* src, int32_t src_sz, 
        void* dst, int32_t dst_sz,
        BAR_PROTO_SEND_METHOD method
    ) {
        std::unique_lock< std::mutex > lock{ _resmtx };
        _BAR_PROTO_RESOLVER& resolver = _resolvers.emplace_back( _BAR_PROTO_RESOLVER{
            _seq: _seq_acq(),
            _dst: dst,
            _sz:  dst_sz,
            _info: info
        } );
        lock.unlock();

        int ret = _send<>( src, src_sz, op, resolver._seq, 0, method );
        _BAR_PROTO_WB__SEND_ASSERT( ret, BAR_PROTO_ERR_SEND );

        return ret;
    }

    void force_waiting_resolvers() {
        std::unique_lock< std::mutex > lock{ _resmtx };
        for( auto& resolver : _resolvers ) {
            resolver._info->ackd = false;
            strcpy( resolver._info->nakr, BAR_PROTO_ERR_STR[ BAR_PROTO_ERR_FORCED ] );
            resolver._info->err = BAR_PROTO_ERR_FORCED;
            resolver._info->sig.store( true, std::memory_order_release );
        #if defined( BAR_PROTO_NOTIFIABLE_ATOMICS )
            resolver._info->sig.notify_one();
        #endif
        }
    }

    bool breach( void ) {
        const char str[] = "BAR_PROTO_BREACH";
        std::unique_lock< std::mutex > lock{ _smtx };
        return _srwrap.send( str, strlen( str ), 0 ) == strlen( str );
    }

    int trust_burst( const void* src, int32_t sz, int flags ) {
        return _srwrap.send( src, sz, flags );
    }

};

