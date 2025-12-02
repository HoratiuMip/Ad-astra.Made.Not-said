#pragma once

#include "core.hpp"
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

class Modbus {
public:
    struct reg_t {
        union {
            struct { uint8_t lo; uint8_t hi; };
            uint16_t w;
        };

        RNK_inline operator uint16_t ( void ) { return w; }
    };

public:
    Modbus() : _device{ ModbusRTUServer } {}

RNK_PROTECTED:
    decltype( ModbusRTUServer )&   _device;

    TaskHandle_t                   _main_tsk   = NULL;
    Atomic< bool >                 _main_act   = { false };

RNK_PROTECTED:
    static void _main( void* arg_ ) {
        auto self = ( Modbus* )arg_;
        self->_main_act.store( true, std::memory_order_release );

    for(; self->_main_act.load( std::memory_order_relaxed );) {
        self->_device.poll();
        taskYIELD();
    }
        vTaskDelete( NULL );
    }

public:
    status_t begin( void ) {
        Uart.begin( MODBUS_UART_BAUD_RATE, MODBUS_UART_CONFIG ); while( not Uart ) taskYIELD();
        Uart.println( TAG" - Modbus server begin." );
        RS485.begin( MODBUS_UART_BAUD_RATE, MODBUS_UART_CONFIG );

    l_begin_attempt:
        int attempt = 1;
        RNK_ASSERT_OR( _device.begin( MODBUS_RIG_DEVICE_ID, MODBUS_UART_BAUD_RATE, MODBUS_UART_CONFIG ) ) {
            Uart.print( TAG" - Modbus server failed. Reattempting... " );
            Uart.print( attempt ); Uart.print( '/' ); Uart.println( MODBUS_SERVER_BEGIN_ATTEMPTS );

            if( MODBUS_SERVER_BEGIN_ATTEMPTS >= ++attempt ) goto l_begin_attempt;

            Uart.println( TAG" - Modbus server failed. Aborting." );
            return -0x1;
        }

        _device.configureHoldingRegisters( 0x0, MODBUS_HOLDING_COUNT );

        RNK_ASSERT_OR( pdPASS == xTaskCreate( 
            &Modbus::_main, MODBUS_MAIN_TASK_NAME, MODBUS_MAIN_TASK_STACK_DEPTH, this, TaskPriority_Current, &_main_tsk
        ) ) return -0x1;

        Uart.println( TAG" - Modbus server started." );
        return 0x0;
    }

public:
    decltype( ModbusRTUServer )* operator -> ( void ) { return &_device; }

};