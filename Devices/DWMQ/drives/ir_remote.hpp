#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include "core.hpp"

#define DECODE_SONY
#include <IRremote.h>


DWMQ_NAMESPACE {


class IR_REMOTE {
public:
    struct PIN_MAP {
        gpio_num_t   I_signal;
    } _pin_map;

    IR_REMOTE( const PIN_MAP& pin_map )
    : _pin_map{ pin_map }
    {
        gpio_reset_pin( _pin_map.I_signal ); 
    }

public:
    status_t init( void ) {
        IrReceiver.begin( _pin_map.I_signal );
        return 0x0;
    }

public:
    std::tuple< ir_signal_t, bool > recv( void ) {
        if( not IrReceiver.decode() ) return { -0x1, false };

        ir_signal_t sig      = 0x0;
        bool        repeated = false; 

        if( IrReceiver.decodedIRData.protocol == SONY ) {
            sig = IrReceiver.decodedIRData.command;

            static ir_signal_t last_sig = 0x0;
            static int64_t     last_us  = 0x0;

            int64_t us = esp_timer_get_time();

            repeated = ( sig == last_sig ) && ( us - last_us < Mirun.Config.IrRemote.BEFORE_ENTRY_US );
            last_us = us; last_sig = sig;
        }

        IrReceiver.resume();
        return { sig, repeated };
    }
};


};

