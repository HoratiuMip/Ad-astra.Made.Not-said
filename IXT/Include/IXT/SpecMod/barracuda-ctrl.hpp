#pragma once
/*
BARRACUDA_CTRL_BUILD_FOR_ON_BOARD
BARRACUDA_CTRL_BUILD_FOR_DRIVER

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

enum PROTO_OP_CODE : int8_t {
    PROTO_OP_CODE_NULL = 0,
    PROTO_OP_CODE_PING = 1,
    PROTO_OP_CODE_SET  = 2,
    PROTO_OP_CODE_GET  = 3,
    PROTO_OP_CODE_DESC = 4,

    _PROTO_OP_CODE_FORCE_BYTE = 0x7f
};

struct proto_head_t {
    proto_head_t() : _dw0{ _b0: 0x42, _b1: 0x41, _b2: 0x52, op: 0x00 }, _dw1{ 0x0 } {}
    union {
        int32_t sig;
        struct{ int8_t _b0, _b1, _b2; int8_t op; } _dw0;
    };
    union {
        int32_t   size;
        int32_t   _dw1;
    };
};
static_assert( sizeof( proto_head_t ) == 2*sizeof( int32_t ) );

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

struct state_desc_t {
    joystick_t      rachel,       samantha;
    /*              |lower left   |upper right */
    switch_t        giselle, karina, ningning, winter;
    /*              |blue    |red    |yellow   |green */
};
static_assert( sizeof( state_desc_t ) == 
    4*sizeof( switch_t )
    +
    2*sizeof( joystick_t )
);


};