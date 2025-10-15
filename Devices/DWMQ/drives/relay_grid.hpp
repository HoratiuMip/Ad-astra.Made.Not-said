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
    } _pin_map;

    RELAY_GRID( const PIN_MAP& pin_map )
    : _pin_map{ pin_map }
    {
        gpio_reset_pin( _pin_map.Q_power ); 
        gpio_reset_pin( _pin_map.Q_blue ); 
        gpio_reset_pin( _pin_map.Q_vol_up ); 
        gpio_reset_pin( _pin_map.Q_vol_down ); 
    }

public:
    status_t init( void ) {
        return 0;
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
    void toggle_power( void ) {
        this->_close( _pin_map.Q_power );
        vTaskDelay( Miruna.Config.RelayGrid.power_hold_ms );
        this->_open( _pin_map.Q_power );
    }

    void blue_enable( void ) {
        this->_close( _pin_map.Q_blue );
    }

    void blue_disable( void ) {
        this->_open( _pin_map.Q_blue );
    }

};


};

