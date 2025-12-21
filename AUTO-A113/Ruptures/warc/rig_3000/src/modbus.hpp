#pragma once

#include "core.hpp"
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

class Modbus {
public:
    Modbus() : _device{ ModbusRTUServer }, _main_mtx{ xSemaphoreCreateMutex() } {}

RNK_PROTECTED:
    decltype( ModbusRTUServer )&   _device;
    
    TaskHandle_t                   _main_tsk   = NULL;
    Atomic< bool >                 _main_act   = { false };
    SemaphoreHandle_t              _main_mtx   = NULL;

RNK_PROTECTED:
    static void _main( void* arg_ ) {
        auto self = ( Modbus* )arg_;
        self->_main_act.store( true, std::memory_order_release );

    for(; self->_main_act.load( std::memory_order_relaxed );) {
        xSemaphoreTake( self->_main_mtx, portMAX_DELAY );
        self->_device.poll();
        xSemaphoreGive( self->_main_mtx );
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

        _device.configureInputRegisters( 0x00, MODBUS_INPUT_COUNT );
        _device.writeInputRegisters( 0x00, ( uint16_t* )"WR3K", 2 );

        RNK_ASSERT_OR( pdPASS == xTaskCreate( 
            &Modbus::_main, MODBUS_MAIN_TASK_NAME, MODBUS_MAIN_TASK_STACK_DEPTH, this, TaskPriority_Current, &_main_tsk
        ) ) return -0x1;

        Uart.println( TAG" - Modbus server started." );
        return 0x0;
    }

public:
    RNK_inline status_t write_input_registers( int addr_, uint16_t* src_, int nb_ ) {
        xSemaphoreTake( _main_mtx, portMAX_DELAY );
        status_t status = _device.writeInputRegisters( addr_, src_, nb_ );
        xSemaphoreGive( _main_mtx );
        return status;
    }

};