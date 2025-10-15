#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include "core.hpp"

#include "HX1838Decoder.h"


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
        return 0;
    }

public:
    std::tuple< ir_signal_t, bool > recv( void ) {
        if( !_decoder.available() ) return { 0x0, false };

        return { _decoder.getDecodedData(), _decoder.isRepeatSignal() };
    }
};


};

