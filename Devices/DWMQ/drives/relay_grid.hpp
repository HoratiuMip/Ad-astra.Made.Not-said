#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#include "core.hpp"


DWMQ_NAMESPACE {


class RELAY_GRID {
public:
    struct PIN_MAP {
        gpio_num_t   Q_power;
        gpio_num_t   Q_blue;
        gpio_num_t   Q_vol_up;
        gpio_num_t   Q_vol_down;
        gpio_num_t   Q_fans;
    } _pin_map;

    RELAY_GRID( const PIN_MAP& pin_map )
    : _pin_map{ pin_map }
    {
        gpio_reset_pin( _pin_map.Q_power ); 
        gpio_reset_pin( _pin_map.Q_blue ); 
        gpio_reset_pin( _pin_map.Q_vol_up ); 
        gpio_reset_pin( _pin_map.Q_vol_down ); 
        gpio_reset_pin( _pin_map.Q_fans ); 
    }

public:
    status_t init( void ) {
        return 0x0;
    }

protected:
    DWMQ_inline void _open( gpio_num_t pin ) {
        gpio_set_direction( pin, GPIO_MODE_INPUT );
    }

    DWMQ_inline void _close( gpio_num_t pin ) {
        gpio_set_direction( pin, GPIO_MODE_OUTPUT );
        gpio_set_level( pin, 0x0 );
    }

public:
    DWMQ_inline void impulse_power( void ) {
        this->_close( _pin_map.Q_power );
        vTaskDelay( Mirun.Config.RelayGrid.POWER_HOLD_MS );
        this->_open( _pin_map.Q_power );
    }

#define _DWMQ_RELAY_GRID_QOPS( field ) \
    DWMQ_inline void engage_##field( void ) { this->_close( _pin_map.Q_##field ); } \
    DWMQ_inline void disengage_##field( void ) { this->_open( _pin_map.Q_##field ); } \
    DWMQ_inline void toggle_##field( void ) { if( 0x1 & ( GPIO.enable >> _pin_map.Q_##field ) ) this->disengage_##field(); else this->engage_##field(); }

    _DWMQ_RELAY_GRID_QOPS( blue )
    _DWMQ_RELAY_GRID_QOPS( vol_up )
    _DWMQ_RELAY_GRID_QOPS( vol_down )
    _DWMQ_RELAY_GRID_QOPS( fans )

};


};

