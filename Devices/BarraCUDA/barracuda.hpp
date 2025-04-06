#pragma once
/*===== BarraCUDA-CTRL - Main header. - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> The quintessential structures of the controller.
|
======*/

namespace barra {

const char* const DEVICE_NAME = "BarraCUDA-CTRL";
const wchar_t* const DEVICE_NAME_W = L"BarraCUDA-CTRL";


struct switch_t {
    uint8_t   dwn       : 1   = 0;
    uint8_t   prs       : 1   = 0;
    uint8_t   rls       : 1   = 0;
    int8_t    _reserved : 5   = 0;
};
static_assert( sizeof( switch_t ) == sizeof( int8_t ) );

struct joystick_t {
    float      x            = 0.0;
    float      y            = 0.0;
    switch_t   sw           = {};
    int8_t     _reserver1   = 0;
    int16_t    _reserved2   = 0;
};
static_assert( sizeof( joystick_t ) == 2*sizeof( float ) + sizeof( switch_t ) + 3 );

struct gyro_t {
    struct { float x, y, z; }   acc         = { 0, 0, 0 };
    struct { float x, y, z; }   gyr         = { 0, 0, 0 };
    int32_t                     _reserved   = 0;
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
    joystick_t        rachel,       samantha;
    /*                |lower left   |upper right */
    switch_t          giselle, karina, ningning, winter;
    /*                |blue    |red    |yellow   |green */

    gyro_t            gran;

    light_sensor_t    naksu;

    potentiometer_t   kazuha;
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