#pragma once
/*
BARRACUDA_CTRL_BUILD_FOR_ON_BOARD_UC
BARRACUDA_CTRL_BUILD_FOR_ENGINE_DRIVER

BARRACUDA_CTRL_ARCHITECTURE_LITTLE
BARRACUDA_CTRL_ARCHITECTURE_BIG
*/

namespace barcud_ctrl {

const char* const DEVICE_NAME = "BarraCUDA-CTRL";
const wchar_t* const DEVICE_NAME_W = L"BarraCUDA-CTRL";


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

struct gyro_t {
    struct { float x, y, z; }   acc        = { 0, 0, 0 };
    struct { float x, y, z; }   gyr        = { 0, 0, 0 };
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