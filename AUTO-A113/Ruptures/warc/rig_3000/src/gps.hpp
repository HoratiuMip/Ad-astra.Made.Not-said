#pragma once

#include "core.hpp"
#include "modbus.hpp"
#include <TinyGPS++.h>

class GPS : public TinyGPSPlus {
#define _GPS_CLAMP_AGE( age ) (([](uint32_t age_)->uint8_t{age_/=1000;return age_>250?250:age_;})(age))

#define _GPS_UPDATE_PACK_ATTR( pck_fld, tiny_fld, tiny_fnc, cast, age_idx )  \
    if( self->tiny_fld.isUpdated() ) {                                       \
        self->pack.pck_fld  = ( cast )self->tiny_fld.tiny_fnc();             \
        self->pack.ages[ age_idx ] = _GPS_CLAMP_AGE( self->tiny_fld.age() ); \
        any_updates = true;                                                  \
    }

public:
    struct pin_map_t {
        pin_t   Q_rx;
        pin_t   Q_tx;
    };

public:
    GPS( const pin_map_t& pin_map, Modbus& modbus_ ) 
    : _pin_map{ pin_map }, _modbus{ modbus_ }
    {}

public:
    GPS_pack_t         pack       = {};

RNK_PROTECTED:
    pin_map_t          _pin_map   = {};

    Modbus&            _modbus;

    HardwareSerial     _uart      = { 0x1 };

    TaskHandle_t       _main_tsk  = NULL;
    Atomic< bool >     _main_act  = { false };

RNK_PROTECTED:
    static void _main( void* arg_ ) {
        auto self = ( GPS* )arg_;
        self->_main_act.store( true, std::memory_order_release );

    for(; self->_main_act.load( std::memory_order_relaxed );) {
        while( self->_uart.available() and not self->encode( self->_uart.read() ) );

        bool any_updates = false;

        _GPS_UPDATE_PACK_ATTR( lock_count, satellites, value, uint16_t, 0x0 )
        _GPS_UPDATE_PACK_ATTR( time, time, value, uint32_t, 0x1 )
        _GPS_UPDATE_PACK_ATTR( date, date, value, uint32_t, 0x2 )
        if( self->location.isUpdated() ) { 
            self->pack.lat         = ( float )self->location.lat();
            self->pack.lng         = ( float )self->location.lng();
            self->pack.ages[ 0x3 ] = _GPS_CLAMP_AGE( self->location.age() );
            any_updates = true; 
        }
        _GPS_UPDATE_PACK_ATTR( alt, altitude, meters, float, 0x4 )
        _GPS_UPDATE_PACK_ATTR( hdop, hdop, hdop, float, 0x5 )
        
        if( any_updates ) self->_modbus.write_input_registers( GPS_MODBUS_INPUT_ADDR, ( uint16_t* )&self->pack, GPS_MODBUS_INPUT_COUNT );
        taskYIELD();
    }
        vTaskDelete( NULL );
    }

public:
    status_t begin( void ) {
        _uart.begin( GPS_DEVICE_UART_BAUD_RATE, GPS_DEVICE_UART_CONFIG, _pin_map.Q_rx, _pin_map.Q_tx ); while( not _uart ) taskYIELD();

        RNK_ASSERT_OR( pdPASS == xTaskCreate( 
            &GPS::_main, GPS_MAIN_TASK_NAME, GPS_MAIN_TASK_STACK_DEPTH, this, TaskPriority_Current, &_main_tsk
        ) ) return -0x1;

        return 0x0;
    }

#undef _GPS_UPDATE_PACK_ATTR
#undef _GPS_CLAMP_AGE
};
