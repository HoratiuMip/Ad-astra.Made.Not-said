#pragma once

#include "core.hpp"
#include "modbus.hpp"
#include <TinyGPS++.h>

class GPS : public TinyGPSPlus {
public:
    struct pin_map_t {
        pin_t   Q_rx;
        pin_t   Q_tx;
    };

public:
    struct holding_layout_t {
        Modbus::reg_t   reg_0; 
    };

public:
    GPS( const pin_map_t& pin_map, Modbus& modbus_ ) 
    : _pin_map{ pin_map }, _modbus{ modbus_ }
    {}

RNK_PROTECTED:
    pin_map_t          _pin_map   = {};

    Modbus&            _modbus;

    HardwareSerial     _uart      = { 0x1 };

    TaskHandle_t       _main_tsk  = NULL;
    Atomic< bool >     _main_act  = { false };

    holding_layout_t   _holding;

RNK_PROTECTED:
    static void _main( void* arg_ ) {
        auto self = ( GPS* )arg_;
        self->_main_act.store( true, std::memory_order_release );

    for(; self->_main_act.load( std::memory_order_relaxed );) {
        while( self->_uart.available() ) self->encode( self->_uart.read() );

        if( self->satellites.isUpdated() || self->hdop.isUpdated() ) {
            self->_holding.reg_0.lo - self->satellites.value();

            auto hdop = self->hdop.hdop();
            self->_holding.reg_0.hi = hdop > 255.0 ? 255 : ( uint8_t )hdop;

            self->_modbus->holdingRegisterWrite( GPS_MODBUS_INPUT_ADDR + 0x00, self->_holding.reg_0 );
        }

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


};
