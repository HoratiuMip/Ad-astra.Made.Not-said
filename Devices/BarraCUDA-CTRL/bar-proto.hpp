#pragma once
/*===== BAR protocol - Vatca "Mipsan" Tudor-Horatiu
|
>
|
======*/
#include <deque>
#include <functional>



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

template< int BUF_SZ >
struct bar_cache_t {
    char                      _buffer[ BUF_SZ ];
    bar_proto_head_t* const   head                = ( bar_proto_head_t* )_buffer;
    void* const               data                = _buffer + sizeof( *head );
    
    bar_cache_t() { *head = bar_proto_head_t{}; }
};



#define BAR_PROTO_GSTBL_READ_WRITE 0
#define BAR_PROTO_GSTBL_READ_ONLY  1
typedef   std::function< bool( void* ) >   bar_proto_gstbl_get_func_t;
typedef   std::function< bool( void* ) >   bar_proto_gstbl_set_func_t;
#define BAR_PROTO_GSTBL_GET_FUNC [] ( void* src ) -> bool
#define BAR_PROTO_GSTBL_SET_FUNC [] ( void* src ) -> bool
struct BAR_PROTO_GSTBL_ENTRY {
  const char* const            str_id;
  void* const                  src;
  const int16_t                sz;
  const bool                   read_only;    
  bar_proto_gstbl_get_func_t   fnc_get;
  bar_proto_gstbl_set_func_t   fnc_set;         
};

struct BAR_PROTO_GSTBL {
  BAR_PROTO_GSTBL_ENTRY*   entries   = nullptr;
  int                      size     = 0;

  BAR_PROTO_GSTBL_ENTRY* search( const char* str_id ) {
    for( int idx = 0; idx < this->size; ++idx ) {
      if( strcmp( entries[ idx ].str_id, str_id ) == 0 ) return &entries[ idx ];
    }
    return nullptr;
  }

};

class BAR_PROTO_STREAM {
public:
    BAR_PROTO_GSTBL   gstbl   = {};

};
