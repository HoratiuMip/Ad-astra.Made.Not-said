#pragma once

namespace warc { namespace rig_3000 {


const char*     TAG_C_STR                = "[ WARC-RIG-3000 ]";

constexpr int   MODBUS_RIG_DEVICE_ID     = 0x3;
constexpr int   MODBUS_UART_BAUD_RATE    = 115200;

constexpr int   GPS_MODBUS_INPUT_ADDR    = 0x02;
constexpr int   GPS_MODBUS_INPUT_COUNT   = 16;

constexpr int   MODBUS_INPUT_COUNT =
    2                      +
    GPS_MODBUS_INPUT_COUNT 
;


struct __attribute__((packed, aligned(2)))  GPS_pack_t {
    uint16_t   lock_count;
    uint32_t   time;
    uint32_t   date;
    float      lat;
    float      lng;
    float      alt;
    float      hdop;
    uint8_t    ages[ 6 ];

};
static_assert( sizeof( GPS_pack_t ) == 2*GPS_MODBUS_INPUT_COUNT );


} }