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
#include "heat_sink.hpp"


DWMQ_NAMESPACE {


class HYPERVISOR {
public:
    struct PIN_MAP {
        gpio_num_t   I_toggle_fans;
    } _pin_map;

    HYPERVISOR( 
        const PIN_MAP& pin_map,  
        IR_REMOTE*     ir_remote,
        RELAY_GRID*    relay_grid,
        HEAT_SINK*     heat_sink
    )
    : _pin_map{ pin_map }
    {
        _ir_remote.device  = ir_remote;
        _relay_grid.device = relay_grid;
        _heat_sink.device  = heat_sink;

        gpio_reset_pin( _pin_map.I_toggle_fans );
    }

protected:
    struct _IR_REMOTE {
        IR_REMOTE*     device   = nullptr;
        TaskHandle_t   handle   = NULL;

        static void _main( void* arg ) {
            auto hv = ( HYPERVISOR* )arg;

            int64_t vol_us = -0x1;

        for(;;) {
            if( vol_us != -0x1 && esp_timer_get_time() - vol_us > Mirun.Config.HV.VOL_KEEP_ALIVE_US ) {
                vol_us = -0x1;
                hv->_relay_grid.device->disengage_vol_up();
                hv->_relay_grid.device->disengage_vol_down();
            }

            auto [ signal, repeated ] = hv->_ir_remote.device->recv();

            if( signal == 0 ) { DWMQ_YIELD_CORE; continue; }

            Mirun.Echo.inf( "Recieved IR signal [0x%x], %s.", signal, repeated ? "repeated" : "entry" );

            switch( signal ) {
                case Mirun.Config.IrRemote.SIG_IMPULSE_POWER: { if( !repeated ) hv->_relay_grid.device->impulse_power(); break; }

                case Mirun.Config.IrRemote.SIG_BLUE_ENGAGE:    { if( !repeated ) hv->_relay_grid.device->engage_blue(); break; }
                case Mirun.Config.IrRemote.SIG_BLUE_DISENGAGE: { if( !repeated ) hv->_relay_grid.device->disengage_blue(); break; }
                case Mirun.Config.IrRemote.SIG_BLUE_TOGGLE:    { if( !repeated ) hv->_relay_grid.device->toggle_blue(); break; }

                case Mirun.Config.IrRemote.SIG_FANS_ENGAGE:    { if( !repeated ) hv->_relay_grid.device->engage_fans(); break; }
                case Mirun.Config.IrRemote.SIG_FANS_DISENGAGE: { if( !repeated ) hv->_relay_grid.device->disengage_fans(); break; }
                case Mirun.Config.IrRemote.SIG_FANS_TOGGLE:    { if( !repeated ) hv->_relay_grid.device->toggle_fans(); break; }

                case Mirun.Config.IrRemote.SIG_VOL_UP: {
                    vol_us = esp_timer_get_time();
                    hv->_relay_grid.device->disengage_vol_down();
                    hv->_relay_grid.device->engage_vol_up();
                    break;
                }
                case Mirun.Config.IrRemote.SIG_VOL_DOWN: {
                    vol_us = esp_timer_get_time();
                    hv->_relay_grid.device->disengage_vol_up();
                    hv->_relay_grid.device->engage_vol_down();
                    break;
                }

                default: {
                    Mirun.Echo.wrn( "Nothing to do with IR signal [0x%x].", signal );
                    break;
                }
            }
        } }

    }   _ir_remote;

    struct _RELAY_GRID {
        RELAY_GRID*    device   = nullptr;
        TaskHandle_t   handle   = NULL;
    }   _relay_grid;

    struct _HEAT_SINK {
        HEAT_SINK*     device   = nullptr;
        TaskHandle_t   handle   = NULL;

        static void _main( void* arg ) {
            auto hv = ( HYPERVISOR* )arg;

        for(;;) {
            Mirun.Echo.inf( "Heat sink temperature reading %.3f*C.", hv->_heat_sink.device->temp() );
            vTaskDelay( 3000 );
        } }

    }   _heat_sink;  

public:
    status_t init( void ) {
        pinMode( _pin_map.I_toggle_fans, INPUT_PULLDOWN );
        attachInterruptArg( _pin_map.I_toggle_fans, &HYPERVISOR::_ISR_I_toggle_fans, ( void* )this, RISING );

        _ir_remote.device->init();
        _relay_grid.device->init();
        _heat_sink.device->init();

        xTaskCreate( &HYPERVISOR::_IR_REMOTE::_main, "HV_IR_REMOTE_MAIN", Mirun.Config.IrRemote.STACK_MAIN, ( void* )this, TaskPriority_Current, &_ir_remote.handle );
        xTaskCreate( &HYPERVISOR::_HEAT_SINK::_main, "HV_HEAT_SINK_MAIN", Mirun.Config.HeatSink.STACK_MAIN, ( void* )this, TaskPriority_Current, &_heat_sink.handle );

        return 0;
    }

protected:
    static void _ISR_I_toggle_fans( void* arg ) {
        static std::atomic_bool flag = { false };
        
        bool mtx = false;
        if( !flag.compare_exchange_strong( mtx, true, std::memory_order_seq_cst ) ) return;

        static struct FILTER_ARG {
            HYPERVISOR*         hv;
            std::atomic_bool*   flag;
        } filter_arg;

        filter_arg.hv = ( HYPERVISOR* )arg;
        filter_arg.flag = &flag;

        xTaskCreate( [] ( void* arg ) -> void {
            auto fa = ( FILTER_ARG* )arg;

            int c = 0;
            for( int n = 1; n <= 100; ++n ) {
                c += gpio_get_level( fa->hv->_pin_map.I_toggle_fans );
                vTaskDelay( 10 );
            }

            if( c >= 80 ) {
                fa->hv->_relay_grid.device->toggle_fans();
            }

            fa->flag->store( false, std::memory_order_release );
            vTaskDelete( NULL );
        }, "HV_MAN_FANS_MAIN", 4096, ( void* )&filter_arg, TaskPriority_Current, NULL );
    }

};


};

