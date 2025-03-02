#pragma once
/*
BARRACUDA_CTRL_BUILD_FOR_ON_BOARD_UC
BARRACUDA_CTRL_BUILD_FOR_ENGINE_DRIVER

BARRACUDA_CTRL_ARCHITECTURE_LITTLE
BARRACUDA_CTRL_ARCHITECTURE_BIG
*/

namespace barracuda_ctrl {

const char* const DEVICE_NAME = "BarraCUDA-CTRL";
const wchar_t* const DEVICE_NAME_W = L"BarraCUDA-CTRL";
#if defined( BARRACUDA_CTRL_ARCHITECTURE_BIG ) 
    const int32_t PROTO_SIG     = 0x42'41'52'00; 
    const int32_t PROTO_SIG_MSK = 0xff'ff'ff'00;
#elif defined( BARRACUDA_CTRL_ARCHITECTURE_LITTLE )
    const int32_t PROTO_SIG     = 0x00'52'41'42;
    const int32_t PROTO_SIG_MSK = 0x00'ff'ff'ff;
#else
    #error "BarraCUDA-CTRL: Endianess of target architecture not specified."
#endif

enum PROTO_OP : int8_t {
    PROTO_OP_NULL    = 0,

    PROTO_OP_ACK     = 1,
    PROTO_OP_NAK     = 2,

    PROTO_OP_PING    = 11,
    PROTO_OP_SET     = 12,
    PROTO_OP_GET     = 13,

    PROTO_OP_DYNAMIC = 101,

    _PROTO_OP_FORCE_BYTE = 0x7f
};

struct proto_head_t {
    inline static int32_t   _seq_cnt   = 0;

    proto_head_t() { _dw0._sig_b0 = 0x42; _dw0._sig_b1 = 0x41, _dw0._sig_b2 = 0x52; _dw0.op = 0; }

    union {
        struct{ int8_t _sig_b0, _sig_b1, _sig_b2; int8_t op; } _dw0;
        int32_t                                                sig;
    };
    struct{ int32_t seq = 0; }                                 _dw1;
    struct { int16_t sz = 0; int16_t _reserved = 0; }          _dw2;

    int32_t acquire_seq( void ) { return _dw1.seq = _seq_cnt++; }

#if defined( BARRACUDA_CTRL_BUILD_FOR_ENGINE_DRIVER )
    int32_t atomic_acquire_seq( void ) { return _dw1.seq = std::atomic_ref< int32_t >( _seq_cnt ).fetch_add( 1, std::memory_order_relaxed ); }
#endif

    bool is_signed( void ) { return ( sig & PROTO_SIG_MSK ) == PROTO_SIG; }
};
static_assert( sizeof( proto_head_t ) == 3*sizeof( int32_t ) );

struct switch_t {
    int8_t   dwn         = 0;
    int8_t   prs         = 0;
    int8_t   rls         = 0;
    int8_t   _reserved   = 0;
};
static_assert( sizeof( switch_t ) == sizeof( int32_t ) );

struct joystick_t {
    switch_t   sw          = {};
    float      x           = 0.0;
    float      y           = 0.0;
    int32_t    _reserved   = 0;
};
static_assert( sizeof( joystick_t ) == sizeof( switch_t ) + 2*sizeof( float ) + sizeof( int32_t ) );

struct dynamic_state_t {
    joystick_t      rachel,       samantha;
    /*              |lower left   |upper right */
    switch_t        giselle, karina, ningning, winter; float f;
    /*              |blue    |red    |yellow   |green */
};
static_assert( sizeof( dynamic_state_t ) == 
    4*sizeof( switch_t )
    +
    2*sizeof( joystick_t ) + 4
);


template< int SZ >
struct out_cache_t {
    uint8_t               _out_cache[ SZ ];
    proto_head_t* const   _out_cache_head    = ( proto_head_t* )_out_cache;
    uint8_t* const        _out_cache_data    = _out_cache + sizeof( *_out_cache_head );
    
    out_cache_t() { *_out_cache_head = proto_head_t{}; }

    virtual int _out_cache_write( void ) = 0;
};


};