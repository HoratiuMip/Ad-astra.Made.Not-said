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
    struct{ int32_t seq = 0; }                                 _dw1;
    struct { int16_t sz = 0; int16_t _reserved = 0; }          _dw2;

    bool is_signed( void ) { return ( sig & BAR_PROTO_SIG_MSK ) == BAR_PROTO_SIG; }
};
static_assert( sizeof( bar_proto_head_t ) == 3*sizeof( int32_t ) );

template< int16_t BUF_SZ >
struct bar_cache_t {
    char                      _buffer[ BUF_SZ ];
    bar_proto_head_t* const   head                = ( bar_proto_head_t* )_buffer;
    void* const               data                = _buffer + sizeof( *head );
    
    bar_cache_t() { *head = bar_proto_head_t{}; }
};



typedef   std::function< const char*( void* ) >            bar_proto_gstbl_get_func_t;
typedef   std::function< const char*( void*, int16_t ) >   bar_proto_gstbl_set_func_t;
#define BAR_PROTO_GSTBL_READ_WRITE    0
#define BAR_PROTO_GSTBL_READ_ONLY     1
#define BAR_PROTO_GSTBL_VARIABLE_SIZE -1
struct BAR_PROTO_GSTBL_ENTRY {
  const char* const            str_id;
  void* const                  src;
  const int16_t                sz;
  const bool                   read_only;    
  bar_proto_gstbl_get_func_t   get;
  bar_proto_gstbl_set_func_t   set;         
};
struct BAR_PROTO_GSTBL {
  BAR_PROTO_GSTBL_ENTRY*   entries   = nullptr;
  int                      size     = 0;

  BAR_PROTO_GSTBL_ENTRY* search( const char* str_id ) {
    for( int idx = 0; idx < size; ++idx ) {
      if( strcmp( entries[ idx ].str_id, str_id ) == 0 ) return &entries[ idx ];
    }
    return nullptr;
  }

};

typedef   std::function< int( const void*, int16_t, int ) >   bar_proto_stream_send_func_t;
typedef   std::function< int( void*, int16_t, int ) >         bar_proto_stream_recv_func_t;
#define BAR_PROTO_STREAM_SEND_LAMBDA ( const void* src, int16_t sz, int flags ) -> int
#define BAR_PROTO_STREAM_RECV_LAMBDA ( void* dst, int16_t sz, int flags ) -> int
struct BAR_PROTO_SRWRAP {
    bar_proto_stream_send_func_t   send    = nullptr;
    bar_proto_stream_recv_func_t   recv    = nullptr;
};

enum BAR_PROTO_STREAM_ERR : int {
    BAR_PROTO_STREAM_ERR_NONE        = 0,
    BAR_PROTO_STREAM_ERR_NOT_SIGNED  = 1,
    BAR_PROTO_STREAM_ERR_RECV        = 2,
    BAR_PROTO_STREAM_ERR_SEND        = 3,
    BAR_PROTO_STREAM_ERR_NO_RESOLVER = 4,
    BAR_PROTO_STREAM_ERR_SEQUENCE    = 5,
    BAR_PROTO_STREAM_ERR_SIZE        = 6
};
const char* BAR_PROTO_STREAM_ERR_STR[] = {
    "BAR_PROTO_STREAM_ERR_NONE",
    "BAR_PROTO_STREAM_ERR_NOT_SIGNED",
    "BAR_PROTO_STREAM_ERR_RECV",
    "BAR_PROTO_STREAM_ERR_SEND",
    "BAR_PROTO_STREAM_ERR_NO_RESOLVER",
    "BAR_PROTO_STREAM_ERR_SEQUENCE",
    "BAR_PROTO_STREAM_ERR_SIZE"
};
struct BAR_PROTO_STREAM_RESOLVE_RECV_INFO {
    bar_proto_head_t       recv_head    = {};
    BAR_PROTO_STREAM_ERR   err          = BAR_PROTO_STREAM_ERR_NONE;
    const char*            nak_reason   = nullptr;
};
struct BAR_PROTO_STREAM_WAIT_BACK_INFO {
    std::atomic_bool       sig   = { false };
    int16_t                sz    = 0;
    BAR_PROTO_STREAM_ERR   err   = BAR_PROTO_STREAM_ERR_NONE;
};
struct _BAR_PROTO_STREAM_RESOLVER {
    int32_t                            _seq   = 0;
    void*                              _dst   = nullptr;
    int16_t                            _sz    = 0;
    BAR_PROTO_STREAM_WAIT_BACK_INFO*   _info   = nullptr;
};
typedef   std::function< int32_t( void ) >   bar_proto_stream_seq_acq_func_t;
#define BAR_PROTO_STREAM_DONT_USE_CACHE 0
#define bAR_PROTO_STREAM_USE_CACHE      1
#define _BAR_PROTO_STREAM_ASSERT( c, e ) if( !( c ) ) { info->err = e; return -1; }
template< int16_t _CACHE_BUF_SZ > struct BAR_PROTO_STREAM {
    BAR_PROTO_GSTBL                            _gstbl       = {};
    BAR_PROTO_SRWRAP                           _srwrap      = {};
    bar_proto_stream_seq_acq_func_t            _seq_acq     = nullptr;
    std::deque< _BAR_PROTO_STREAM_RESOLVER >   _resolvers   = {};
    std::mutex                                 _resmtx      = {};
    bar_cache_t< _CACHE_BUF_SZ >               _cache       = {};

    void bind_gstbl( const BAR_PROTO_GSTBL& gstbl ) { _gstbl = gstbl; }
    void bind_srwrap( const BAR_PROTO_SRWRAP& srwrap ) { _srwrap = srwrap; }
    void bind_seq_acq( const bar_proto_stream_seq_acq_func_t& seq_acq ) { _seq_acq = seq_acq; }

    bool _cache_send( void ) {
        int16_t sz = sizeof( bar_proto_head_t ) + _cache.head->_dw2.sz; 
        return _srwrap.send( &_cache._buffer, sz, 0 ) == sz;
    }

    int resolve_recv( BAR_PROTO_STREAM_RESOLVE_RECV_INFO* info ) {
        if( int ret = _srwrap.recv( &info->recv_head, sizeof( info->recv_head ), 0 ); ret != sizeof( info->recv_head ) ) {
            info->err = BAR_PROTO_STREAM_ERR_RECV;
            return ret;
        }

        _BAR_PROTO_STREAM_ASSERT( info->recv_head.is_signed(), BAR_PROTO_STREAM_ERR_NOT_SIGNED );

        switch( info->recv_head._dw0.op ) {
            case BAR_PROTO_OP_PING: {
                _cache.head->_dw0.op  = BAR_PROTO_OP_ACK;
                _cache.head->_dw1.seq = info->recv_head._dw1.seq;
                _cache.head->_dw2.sz  = 0;

                _BAR_PROTO_STREAM_ASSERT( _cache_send(), BAR_PROTO_STREAM_ERR_SEND );
            break; }

            case BAR_PROTO_OP_GET: [[fallthrough]];
            case BAR_PROTO_OP_SET: {
                _cache.head->_dw1.seq = info->recv_head._dw1.seq;

                if( info->recv_head._dw2.sz <= 0 ) { info->nak_reason = "BAR_PROTO_NAKR_BAD_FORMAT"; goto l_nak; }
                char buffer[ info->recv_head._dw2.sz ];
                _BAR_PROTO_STREAM_ASSERT( _srwrap.recv( buffer, info->recv_head._dw2.sz, 0 ) == info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_RECV );

                char* delim = buffer - 1; while( *++delim != '\0' ) {
                    if( delim - buffer >= info->recv_head._dw2.sz - 1 ) { info->nak_reason = "BAR_PROTO_NAKR_BAD_FORMAT"; goto l_nak; }
                }

                BAR_PROTO_GSTBL_ENTRY* entry = _gstbl.search( buffer );
                if( entry == nullptr ) { info->nak_reason = "BAR_PROTO_NAKR_NO_ENTRY"; goto l_nak; }

                if( info->recv_head._dw0.op == BAR_PROTO_OP_SET ) goto l_set;
            l_get:
                if( entry->get != nullptr ) {
                    if( const char* nak_reason = entry->get( _cache.data ); nak_reason != nullptr ) { 
                        info->nak_reason = nak_reason; goto l_nak; 
                    }
                } else if( entry->src != nullptr ) {
                    memcpy( _cache.data, entry->src, entry->sz );
                } else {
                    info->nak_reason = "BAR_PROTO_NAKR_NO_PROCEDURE"; goto l_nak;
                }
                _cache.head->_dw2.sz = entry->sz;
                goto l_ack;

            l_set:
                if( entry->read_only ) { info->nak_reason = "BAR_PROTO_NAKR_READ_ONLY"; goto l_nak; }

                int16_t sz = info->recv_head._dw2.sz - ( delim - buffer + 1 );
                if( entry->sz != BAR_PROTO_GSTBL_VARIABLE_SIZE && entry->sz != sz ) { 
                    info->nak_reason = "BAR_PROTO_NAKR_SIZE_MISMATCH"; goto l_nak; 
                }

                if( entry->set != nullptr ) {
                    if( const char* nak_reason = entry->set( delim + 1, sz ); nak_reason != nullptr ) {
                        info->nak_reason = nak_reason; goto l_nak; 
                    }
                } else if( entry->src != nullptr ) {
                    memcpy( entry->src, delim + 1, sz );
                } else {
                    info->nak_reason = "BAR_PROTO_NAKR_NO_PROCEDURE"; goto l_nak;
                }
                _cache.head->_dw2.sz = 0;
                goto l_ack;

            break; }

            l_ack: {
                _cache.head->_dw0.op = BAR_PROTO_OP_ACK;
                _BAR_PROTO_STREAM_ASSERT( _cache_send(), BAR_PROTO_STREAM_ERR_SEND );
            break; }

            l_nak: {
                _cache.head->_dw0.op = BAR_PROTO_OP_NAK;
                
                if( info->nak_reason != nullptr ) {
                    _cache.head->_dw2.sz = strlen( strcpy( ( char* )_cache.data, info->nak_reason ) );
                } else {
                    _cache.head->_dw2.sz = 0;
                }

                _BAR_PROTO_STREAM_ASSERT( _cache_send(), BAR_PROTO_STREAM_ERR_SEND );

            break; }

            case BAR_PROTO_OP_ACK: {
                _BAR_PROTO_STREAM_ASSERT( !_resolvers.empty(), BAR_PROTO_STREAM_ERR_NO_RESOLVER );

                _BAR_PROTO_STREAM_RESOLVER& resolver = _resolvers.front();

                _BAR_PROTO_STREAM_ASSERT( info->recv_head._dw1.seq == resolver._seq, BAR_PROTO_STREAM_ERR_SEQUENCE );

                if( resolver._dst == nullptr ) goto l_resolver_sig;

                _BAR_PROTO_STREAM_ASSERT( resolver._sz >= info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_SIZE );

                _BAR_PROTO_STREAM_ASSERT( _srwrap.recv( resolver._dst, info->recv_head._dw2.sz, 0 ) == info->recv_head._dw2.sz, BAR_PROTO_STREAM_ERR_RECV );

                resolver._info->sz = info->recv_head._dw2.sz;

            l_resolver_sig:
                BAR_PROTO_STREAM_WAIT_BACK_INFO* wb_info = resolver._info;

                std::unique_lock< std::mutex > lock{ _resmtx };
                _resolvers.pop_front();
                lock.unlock();

                wb_info->sig.store( true, std::memory_order_release );
            #if defined( BAR_PROTO_NOTIFIABLE_ATOMICS )
                wb_info->sig.notify_one();
            #endif

            break; }

        }
        
        return 0;
    }

    int wait_back( 
        BAR_PROTO_STREAM_WAIT_BACK_INFO* info, 
        BAR_PROTO_OP op, 
        const void* src, int16_t src_sz, 
        void* dst, int16_t dst_sz,
        bool use_cache 
    ) {
        std::unique_lock< std::mutex > lock{ _resmtx };
        _BAR_PROTO_STREAM_RESOLVER& resolver = _resolvers.emplace_back( _BAR_PROTO_STREAM_RESOLVER{
            _seq: _seq_acq(),
            _dst: dst,
            _sz:  dst_sz,
            _info: info
        } );
        lock.unlock();

        if( use_cache ) {
            _cache.head->_dw0.op  = op;
            _cache.head->_dw1.seq = resolver._seq;
            _cache.head->_dw2.sz  = src_sz;

            memcpy( _cache.data, src, src_sz );

            _BAR_PROTO_STREAM_ASSERT( _cache_send(), BAR_PROTO_STREAM_ERR_SEND );
        } else {
            bar_proto_head_t out_head;
            out_head._dw0.op  = op;
            out_head._dw1.seq = resolver._seq;
            out_head._dw2.sz  = src_sz;

            _BAR_PROTO_STREAM_ASSERT( _srwrap.send( &out_head, sizeof( out_head ), 0 ) == sizeof( out_head ), BAR_PROTO_STREAM_ERR_SEND );
            _BAR_PROTO_STREAM_ASSERT( _srwrap.send( src, src_sz, 0 ) == src_sz, BAR_PROTO_STREAM_ERR_SEND );
        }

        return 0;
    }
};

