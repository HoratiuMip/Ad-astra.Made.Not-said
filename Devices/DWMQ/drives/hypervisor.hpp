#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include "core.hpp"
#include "ir_remote.hpp"
#include "relay_grid.hpp"


DWMQ_NAMESPACE {


class HYPERVISOR {
public:
    struct PIN_MAP {
    
    } _pin_map;

    HYPERVISOR( 
        const PIN_MAP& pin_map,  
        IR_REMOTE*     ir_remote,
        RELAY_GRID*    relay_grid
    )
    : _pin_map{ pin_map }
    {
        _ir_remote.device  = ir_remote;
        _relay_grid.device = relay_grid;
    }

protected:
    struct _IR_REMOTE {
        IR_REMOTE*     device   = nullptr;
        TaskHandle_t   handle   = NULL;

        static void _main( void* arg ) {
            auto hv = ( HYPERVISOR* )arg;

        for(;;) {
            auto [ signal, repeated ] = hv->_ir_remote.device->recv();

            if( signal == 0 ) { DWMQ_YIELD_CORE; continue; }

            Miruna.Echo.inf( "Processing IR signal [%x].", signal );

            switch( signal ) {
                case Miruna.Config.IrRemote.signal_toggle_power: if( !repeated ) hv->_relay_grid.device->toggle_power(); break;
                case Miruna.Config.IrRemote.signal_blue_enable: if( !repeated ) hv->_relay_grid.device->blue_enable(); break;
                case Miruna.Config.IrRemote.signal_blue_disable: if( !repeated ) hv->_relay_grid.device->blue_disable(); break;

                default: continue;
            }

            vTaskDelay( Miruna.Config.IrRemote.after_signal_hold_ms );
        } }

    }   _ir_remote;

    struct _RELAY_GRID {
        RELAY_GRID*    device   = nullptr;
        TaskHandle_t   handle   = NULL;
    }   _relay_grid;

public:
    status_t init( void ) {
        _ir_remote.device->init();
        _relay_grid.device->init();

        xTaskCreate( &HYPERVISOR::_IR_REMOTE::_main, "HV_MAIN_IR_REMOTE", Miruna.Config.IrRemote.stack_main, ( void* )this, TaskPriority_Current, &_ir_remote.handle );

        return 0;
    }

protected:


};


};

