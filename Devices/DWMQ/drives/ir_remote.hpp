#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include "core.hpp"

#include <HX1838Decoder.h>


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
        _decoder = IRDecoder{ _pin_map.I_signal };
    }

protected:
    IRDecoder   _decoder   = { 0 };

public:
    status_t init( void ) {
        _decoder.begin();
        return 0x0;
    }

public:
    std::tuple< ir_signal_t, bool > recv( void ) {
        if( !_decoder.available() ) return { 0x0, false };

        static ir_signal_t last_sig = 0x0;
        static int64_t     last_us  = 0x0;

        ir_signal_t sig = _decoder.getDecodedData();
        for(;_decoder.available();) sig = _decoder.getDecodedData();

        int64_t     us  = esp_timer_get_time();

        bool repeated = ( sig == last_sig ) && ( us - last_us < Mirun.Config.IrRemote.BEFORE_ENTRY_US );
        last_us = us; last_sig = sig;
        return { sig, repeated };
    }
};


};

