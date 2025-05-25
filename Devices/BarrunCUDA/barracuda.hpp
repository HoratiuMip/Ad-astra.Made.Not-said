#pragma once
/*===== BarraCUDA-CTRL - Main header. - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> The quintessential structures of the controller.
|
======*/

namespace barra {

const char* const DEVICE_NAME = "BarrunCUDA";
const wchar_t* const DEVICE_NAME_W = L"BarrunCUDA";

constexpr float JS_HYST_LOW = 0.8;
constexpr float JS_HYST_HIGH = 0.9;


struct switch_t {
    uint8_t   dwn       : 1;
    uint8_t   prs       : 1;
    uint8_t   rls       : 1;
    int8_t    _reserved : 5;
};
static_assert( sizeof( switch_t ) == sizeof( int8_t ) );

struct joystick_t {
    float      x;
    float      y;
    switch_t   sw;
    struct {
        int8_t   x         : 2;
        int8_t   y         : 2;
        int8_t   is        : 2;
        int8_t   _reserved : 2;
    }          edg;
    int16_t    _reserved;
};
static_assert( sizeof( joystick_t ) == 2*sizeof( float ) + sizeof( switch_t ) + 3 );

struct gyro_t {
    struct { float x, y, z; }   acc;
    struct { float x, y, z; }   gyr;
    int32_t                     _reserved;
};
static_assert( sizeof( gyro_t ) == 6*sizeof( float ) + sizeof( int32_t ) );

struct light_sensor_t {
    float   lvl;
};
static_assert( sizeof( light_sensor_t ) == sizeof( float ) );

struct potentiometer_t {
    float   lvl;
};
static_assert( sizeof( potentiometer_t ) == sizeof( float ) ); 

struct dynamic_t {
union {
    struct { joystick_t        rachel,       samantha; };
    /*                         |lower left   |upper right */
    joystick_t joysticks[ 2 ];
};

union {
    struct { switch_t giselle, karina, ningning, winter; };
    /*                |blue    |red    |yellow   |green */
    switch_t switches[ 4 ];
};

    gyro_t            gran;

    light_sensor_t    naksu;

    potentiometer_t   tanya;
};
static_assert( sizeof( dynamic_t ) == 
    4*sizeof( switch_t )
    +
    2*sizeof( joystick_t )
    +
    sizeof( gyro_t )
    +
    sizeof( light_sensor_t )
    +
    sizeof( potentiometer_t )
);



};