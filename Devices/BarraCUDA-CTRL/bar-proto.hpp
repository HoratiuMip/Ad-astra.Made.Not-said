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
#define BAR_PROTO_GSTBL_READ_WRITE    0
#define BAR_PROTO_GSTBL_READ_ONLY     1
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
        if( idx > size || idx < 0 ) return nullptr;
        return &entries[ idx ];
    }
};

typedef   std::function< int( const void*, int32_t, int ) >   bar_proto_stream_send_func_t;
typedef   std::function< int( void*, int32_t, int ) >         bar_proto_stream_recv_func_t;
#define BAR_PROTO_STREAM_SEND_LAMBDA ( const void* src, int32_t sz, int flags ) -> int
#define BAR_PROTO_STREAM_RECV_LAMBDA ( void* dst, int32_t sz, int flags ) -> int
struct BAR_PROTO_SRWRAP {
    bar_proto_stream_send_func_t   send    = nullptr;
    bar_proto_stream_recv_func_t   recv    = nullptr;
};

enum BAR_PROTO_STREAM_SEND_METHOD {
    BAR_PROTO_STREAM_SEND_METHOD_DIRECT,
    BAR_PROTO_STREAM_SEND_METHOD_COPY_TO_CACHE,
    BAR_PROTO_STREAM_SEND_METHOD_COPY_ON_STACK,
    BAR_PROTO_STREAM_SEND_METHOD_AUTO
};
enum BAR_PROTO_STREAM_ERR : int {
    BAR_PROTO_STREAM_ERR_NONE        = 0,
    BAR_PROTO_STREAM_ERR_NOT_SIGNED  = 1,
    BAR_PROTO_STREAM_ERR_RECV        = 2,
    BAR_PROTO_STREAM_ERR_SEND        = 3,
    BAR_PROTO_STREAM_ERR_NO_RESOLVER = 4,
    BAR_PROTO_STREAM_ERR_SEQUENCE    = 5,
    BAR_PROTO_STREAM_ERR_SIZE        = 6,
    BAR_PROTO_STREAM_ERR_NAKR_SIZE   = 7,
    BAR_PROTO_STREAM_ERR_CONN_RESET  = 8,
    BAR_PROTO_STREAM_ERR_NO_ENTRY    = 9
};
const char* BAR_PROTO_STREAM_ERR_STR[] = {
    "BAR_PROTO_STREAM_ERR_NONE",
    "BAR_PROTO_STREAM_ERR_NOT_SIGNED",
    "BAR_PROTO_STREAM_ERR_RECV",
    "BAR_PROTO_STREAM_ERR_SEND",
    "BAR_PROTO_STREAM_ERR_NO_RESOLVER",
    "BAR_PROTO_STREAM_ERR_SEQUENCE",
    "BAR_PROTO_STREAM_ERR_SIZE",
    "BAR_PROTO_STREAM_ERR_NAKR_SIZE",
    "BAR_PROTO_STREAM_ERR_CONN_RESET",
    "BAR_PROTO_STREAM_ERR_NO_ENTRY"
};
struct BAR_PROTO_STREAM_RESOLVE_RECV_INFO {
    bar_proto_head_t       recv_head   = {};
    int32_t                sent        = 0;
    BAR_PROTO_STREAM_ERR   err         = BAR_PROTO_STREAM_ERR_NONE;
    const char*            nakr        = nullptr;
};
#define BAR_PROTO_STREAM_NAKR_MAX_SIZE 32
struct BAR_PROTO_STREAM_WAIT_BACK_INFO {
    std::atomic_bool       sig                                          = { false };
    int32_t                sz                                           = 0;
    bool                   ackd                                         = false;
    BAR_PROTO_STREAM_ERR   err                                          = BAR_PROTO_STREAM_ERR_NONE;
    char                   nakr[ BAR_PROTO_STREAM_NAKR_MAX_SIZE + 1 ]   = { '\0' };
};
struct _BAR_PROTO_STREAM_RESOLVER {
    int16_t                            _seq    = 0;
    void*                              _dst    = nullptr;
    int32_t                            _sz     = 0;
    BAR_PROTO_STREAM_WAIT_BACK_INFO*   _info   = nullptr;
};
typedef   std::function< int16_t( void ) >   bar_proto_stream_seq_acq_func_t;
#define _BAR_PROTO_STREAM_INFO_ASSERT( c, e ) if( !( c ) ) { info->err = e; return -1; }
#define _BAR_PROTO_STREAM_INFO__SEND_ASSERT( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? BAR_PROTO_STREAM_ERR_CONN_RESET : e; return _sret; } info->sent += _sret; }
#define _BAR_PROTO_STREAM_INFO__SEND_ASSERT_NO_ADD( s, e ) { int _sret = ( s ); if( _sret <= 0 ) { info->err = ( _sret == 0 ) ? BAR_PROTO_STREAM_ERR_CONN_RESET : e; return _sret; } }
#define _BAR_PROTO_STREAM_INFO_NAK_IF( c, r ) if( c ) { info->nakr = "BAR_PROTO_NAKR_" r; goto l_gs_nak; }
#define _BAR_PROTO_STREAM_INFO_RECV_ASSERT( r, t, e ) { int _rret = ( r ); if( _rret <= 0 ) { info->err = ( _rret == 0 ) ? BAR_PROTO_STREAM_ERR_CONN_RESET : e; return _rret; } if( _rret != ( t ) ) return -1; }
#define _BAR_PROTO_STREAM__SEND_ASSERT_RET( s, t ) { int _sret = ( s ); if( _sret <= 0 ) return _sret; if( _sret != ( t ) ) return -1; ret += _sret; }
template< int16_t _CACHE_BUF_SZ > struct BAR_PROTO_STREAM {
    BAR_PROTO_GSTBL                            _gstbl       = {};
    BAR_PROTO_BRSTBL                           _brstbl      = {};
    BAR_PROTO_SRWRAP                           _srwrap      = {};
    bar_proto_stream_seq_acq_func_t            _seq_acq     = nullptr;
    std::deque< _BAR_PROTO_STREAM_RESOLVER >   _resolvers   = {};
    std::mutex                                 _resmtx      = {};
    bar_cache_t< _CACHE_BUF_SZ >               _cache       = {};

    void bind_gstbl( const BAR_PROTO_GSTBL& gstbl ) { _gstbl = gstbl; }
    void bind_brstbl( const BAR_PROTO_BRSTBL& brstbl ) { _brstbl = brstbl; }
    void bind_srwrap( const BAR_PROTO_SRWRAP& srwrap ) { _srwrap = srwrap; }
    void bind_seq_acq( const bar_proto_stream_seq_acq_func_t& seq_acq ) { _seq_acq = seq_acq; }

    int _send( const void* src, int32_t sz, BAR_PROTO_OP op, int16_t seq, int flags, BAR_PROTO_STREAM_SEND_METHOD method ) {
        int ret = 0;

        switch( method ) {
        l_direct:
            case BAR_PROTO_STREAM_SEND_METHOD_DIRECT: {
                bar_proto_head_t head;
                head._dw0.op  = op;
                head._dw1.seq = seq;
                head._dw2.sz  = sz;
                _BAR_PROTO_STREAM__SEND_ASSERT_RET( _srwrap.send( &head, sizeof( head ), flags ), sizeof( head ) );
                
                if( src != nullptr )
                    _BAR_PROTO_STREAM__SEND_ASSERT_RET( _srwrap.send( src, sz, flags ), sz );
            break; }
        
        l_copy_to_cache:
            case BAR_PROTO_STREAM_SEND_METHOD_COPY_TO_CACHE: {
                std::unique_lock< decltype( _cache.mtx ) > lock{ _cache.mtx };
                _cache.head->_dw0.op  = op;
                _cache.head->_dw1.seq = seq;
                _cache.head->_dw2.sz  = sz;
                if( src != nullptr )
                    memcpy( _cache.data, src, sz );
                int32_t tot_sz = sizeof( bar_proto_head_t ) + sz;
                _BAR_PROTO_STREAM__SEND_ASSERT_RET( _srwrap.send( _cache._buffer, tot_sz, flags ), tot_sz );
            break; }

        l_copy_on_stack:
            case BAR_PROTO_STREAM_SEND_METHOD_COPY_ON_STACK: {
                int32_t tot_sz = sizeof( bar_proto_head_t ) + sz;
                char buffer[ tot_sz ];
                *( bar_proto_head_t* )buffer = bar_proto_head_t{};
                if( src != nullptr )
                    memcpy( buffer + sizeof( bar_proto_head_t ), src, sz );
                ( ( bar_proto_head_t* )buffer )->_dw0.op  = op;
                ( ( bar_proto_head_t* )buffer )->_dw1.seq = seq;
                ( ( bar_proto_head_t* )buffer )->_dw2.sz  = sz;
                   
                _BAR_PROTO_STREAM__SEND_ASSERT_RET( _srwrap.send( buffer, tot_sz, flags ), tot_sz );
            break; }

            case BAR_PROTO_STREAM_SEND_METHOD_AUTO: {
                if( src == nullptr )
                    goto l_direct;
                else if( sz + sizeof( bar_proto_head_t ) <= 64  )
                    goto l_copy_on_stack;
                else if( sz + sizeof( bar_proto_head_t ) <= _CACHE_BUF_SZ )
                    goto l_copy_to_cache;
                else 
                    goto l_direct;
            break; }
        }

        return ret;
    }

    int resolve_recv( BAR_PROTO_STREAM_RESOLVE_RECV_INFO* info ) {
        if( int ret = _srwrap.recv( &info->recv_head, sizeof( info->recv_head ), 0 ); ret != sizeof( bar_proto_head_t ) ) {
            info->err = ( ret == 0 ) ? BAR_PROTO_STREAM_ERR_CONN_RESET : BAR_PROTO_STREAM_ERR_RECV;
            return ret;
        }

        _BAR_PROTO_STREAM_INFO_ASSERT( info->recv_head.is_signed(), BAR_PROTO_STREAM_ERR_NOT_SIGNED );

        switch( info->recv_head._dw0.op ) {
            case BAR_PROTO_OP_PING: {
                _BAR_PROTO_STREAM_INFO__SEND_ASSERT( 
                    _send( nullptr, 0, BAR_PROTO_OP_ACK, info->recv_head._dw1.seq, 0, BAR_PROTO_STREAM_SEND_METHOD_DIRECT ), 
                    BAR_PROTO_STREAM_ERR_SEND 
                );
            break; }

            case BAR_PROTO_OP_GET: [[fallthrough]];
            case BAR_PROTO_OP_SET: {
                _BAR_PROTO_STREAM_INFO_NAK_IF( info->recv_head._dw2.sz <= 0, "BAD_SIZE" );
                char buffer[ info->recv_head._dw2.sz ];
                _BAR_PROTO_STREAM_INFO_RECV_ASSERT(
                    _srwrap.recv( buffer, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_RECV
                );

                char* delim = buffer - 1; while( *++delim != '\0' ) {
                    _BAR_PROTO_STREAM_INFO_NAK_IF( delim - buffer >= info->recv_head._dw2.sz - 1, "BAD_FORMAT" );
                }

                BAR_PROTO_GSTBL_ENTRY* entry = _gstbl.search( buffer );
                _BAR_PROTO_STREAM_INFO_NAK_IF( entry == nullptr, "NO_ENTRY" );

                if( info->recv_head._dw0.op == BAR_PROTO_OP_SET ) goto l_set;
            l_get:
                if( entry->src != nullptr ) {
                    _BAR_PROTO_STREAM_INFO__SEND_ASSERT( 
                        _send( entry->src, entry->sz, BAR_PROTO_OP_ACK, info->recv_head._dw1.seq, 0, BAR_PROTO_STREAM_SEND_METHOD_AUTO ),
                        BAR_PROTO_STREAM_ERR_SEND
                    );
                } else {
                    _BAR_PROTO_STREAM_INFO_NAK_IF( true, "NO_PROCEDRE" );
                }
            break;
            l_set:
                _BAR_PROTO_STREAM_INFO_NAK_IF( entry->read_only, "READ_ONLY" );

                int32_t sz = info->recv_head._dw2.sz - ( delim - buffer + 1 );
                _BAR_PROTO_STREAM_INFO_NAK_IF( entry->sz != sz, "SIZE_MISMATCH" );

                if( entry->set != nullptr ) {
                    if( const char* nakr = entry->set( delim + 1, sz ); nakr != nullptr ) {
                        info->nakr = nakr; goto l_gs_nak; 
                    }
                } else if( entry->src != nullptr ) {
                    memcpy( entry->src, delim + 1, sz );
                } else {
                    _BAR_PROTO_STREAM_INFO_NAK_IF( true, "NO_PROCEDRE" );
                }
                _BAR_PROTO_STREAM_INFO__SEND_ASSERT( 
                    _send( nullptr, 0, BAR_PROTO_OP_ACK, info->recv_head._dw1.seq, 0, BAR_PROTO_STREAM_SEND_METHOD_DIRECT ),
                    BAR_PROTO_STREAM_ERR_SEND
                );
            break; }

            l_gs_nak: {
                _BAR_PROTO_STREAM_INFO__SEND_ASSERT(
                    _send( info->nakr, info->nakr ? strlen( info->nakr ) + 1 : 0, BAR_PROTO_OP_NAK, info->recv_head._dw1.seq, 0, BAR_PROTO_STREAM_SEND_METHOD_COPY_ON_STACK ),
                    BAR_PROTO_STREAM_ERR_SEND 
                );

            break; }

            case BAR_PROTO_OP_NAK: [[fallthrough]];
            case BAR_PROTO_OP_ACK: {
                _BAR_PROTO_STREAM_INFO_ASSERT( !_resolvers.empty(), BAR_PROTO_STREAM_ERR_NO_RESOLVER );

                _BAR_PROTO_STREAM_RESOLVER& resolver = _resolvers.front();

                _BAR_PROTO_STREAM_INFO_ASSERT( info->recv_head._dw1.seq == resolver._seq, BAR_PROTO_STREAM_ERR_SEQUENCE );

                BAR_PROTO_STREAM_WAIT_BACK_INFO* wb_info = resolver._info;

                if( info->recv_head._dw0.op == BAR_PROTO_OP_NAK ) {
                    _BAR_PROTO_STREAM_INFO_ASSERT( info->recv_head._dw2.sz <= BAR_PROTO_STREAM_NAKR_MAX_SIZE, BAR_PROTO_STREAM_ERR_NAKR_SIZE );
                    _BAR_PROTO_STREAM_INFO_RECV_ASSERT(
                        _srwrap.recv( wb_info->nakr, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_RECV
                    );
                    goto l_resolver_sig;
                }

                if( resolver._dst == nullptr ) goto l_resolver_sig;

                _BAR_PROTO_STREAM_INFO_ASSERT( resolver._sz >= info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_SIZE );
                _BAR_PROTO_STREAM_INFO_RECV_ASSERT(
                    _srwrap.recv( resolver._dst, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_RECV
                );

                wb_info->sz = info->recv_head._dw2.sz;

            l_resolver_sig:
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
                _BAR_PROTO_STREAM_INFO_ASSERT( entry != nullptr, BAR_PROTO_STREAM_ERR_NO_ENTRY );
                _BAR_PROTO_STREAM_INFO_ASSERT( info->recv_head._dw2.sz <= entry->sz, BAR_PROTO_STREAM_ERR_SIZE );

                _BAR_PROTO_STREAM_INFO_RECV_ASSERT(
                    _srwrap.recv( entry->dst, info->recv_head._dw2.sz, 0 ), info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_RECV
                );
            break; }

        }
        
        return sizeof( bar_proto_head_t ) + info->recv_head._dw2.sz;
    }

    int wait_back( 
        BAR_PROTO_STREAM_WAIT_BACK_INFO* info, 
        BAR_PROTO_OP op, 
        const void* src, int32_t src_sz, 
        void* dst, int32_t dst_sz,
        BAR_PROTO_STREAM_SEND_METHOD method
    ) {
        std::unique_lock< std::mutex > lock{ _resmtx };
        _BAR_PROTO_STREAM_RESOLVER& resolver = _resolvers.emplace_back( _BAR_PROTO_STREAM_RESOLVER{
            _seq: _seq_acq(),
            _dst: dst,
            _sz:  dst_sz,
            _info: info
        } );
        lock.unlock();

        _BAR_PROTO_STREAM_INFO__SEND_ASSERT_NO_ADD(
            _send( src, src_sz, op, resolver._seq, 0, method ),
            BAR_PROTO_STREAM_ERR_SEND 
        );

        return 0;
    }

    int trust_burst( const void* src, int32_t sz, int flags ) {
        return _srwrap.send( src, sz, flags );
    }

};

