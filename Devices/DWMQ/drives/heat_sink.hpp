#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include "core.hpp"

#include <Adafruit_BMP280.h>


DWMQ_NAMESPACE {


class HEAT_SINK {
public:
    struct PIN_MAP {
    } _pin_map;

    HEAT_SINK( const PIN_MAP& pin_map )
    : _pin_map{ pin_map }
    {
        
    }

protected:
    Adafruit_BMP280   _temp_sensor   = {};

public:
    status_t init( void ) {
        new ( &_temp_sensor ) Adafruit_BMP280{ Mirun.Config.I2C_bus };

        if( false == _temp_sensor.begin( 118 ) ) {
            Mirun.Echo.err( "Fault engaging the heat sink temperature sensor." );
            return -0x1;
        }

        Mirun.Echo.inf( "Engaged the heat sink temperature sensor." );

        return 0x0;
    }

public:
    float temp( void ) { return _temp_sensor.readTemperature(); }

};


};

