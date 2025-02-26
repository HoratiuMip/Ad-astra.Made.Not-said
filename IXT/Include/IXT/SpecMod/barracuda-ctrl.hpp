#pragma once
/*
Define BARRACUDA_CTRL_BUILD_FOR_ON_BOARD when building for the controller.
*/

namespace barracuda_ctrl {

const char* const DEVICE_NAME = "BarraCUDA";
const int32_t PROTO_SIG = 0x42'41'52'00;
enum PROTO_OP_CODE : int8_t {
    PROTO_OP_CODE_NULL = 0,
    PROTO_OP_CODE_PING = 1,
    PROTO_OP_CODE_SET  = 2,
    PROTO_OP_CODE_GET  = 3,
    PROTO_OP_CODE_DESC = 4,

    _PROTO_OP_CODE_FORCE_BYTE = 0x7f
};

struct proto_head_t {
    proto_head_t() : _dw0{ 0 }, _dw1{ 0 } {}
    union {
        const int32_t sig = PROTO_SIG;
        struct{ const int8_t _b0, _b1, _b2; int8_t op; } _dw0;
    };
    union {
        int32_t   size = 0;
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