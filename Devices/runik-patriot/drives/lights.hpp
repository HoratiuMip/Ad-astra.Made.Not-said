#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"

namespace rp {


class Light_drive {
public:
    const struct PIN_MAP {
        rnk::pin_t   Q_left_headlight;
        rnk::pin_t   Q_right_headlight;;
    } _pin_map;

public:
    inline static constexpr int   PWM_TOP   = 255;

public:
    Light_drive( const PIN_MAP& pin_map_ )
    : _pin_map{ pin_map_ }
    {}

public:
    rnk::status_t begin( void ) {
        pinMode( _pin_map.Q_left_headlight, INPUT );
        pinMode( _pin_map.Q_right_headlight, INPUT );

        return 0x0;
    }
    
    RNK_inline int pwr_2_pwm( float pwr_ ) {
        return pwr_ * PWM_TOP;
    }

public:
    void right_headlight( float pwr_ ) {
        if( pwr_ == 0.0 ) { pinMode( _pin_map.Q_right_headlight, INPUT ); return; }
        pinMode( _pin_map.Q_right_headlight, OUTPUT );
        if( pwr_ == 1.0 ) { digitalWrite( _pin_map.Q_right_headlight, 0x1 ); return; }
        analogWrite( _pin_map.Q_right_headlight, this->pwr_2_pwm( pwr_ ) );
    }

    void left_headlight( float pwr_ ) {
        if( pwr_ == 0.0 ) { pinMode( _pin_map.Q_left_headlight, INPUT ); return; }
        pinMode( _pin_map.Q_left_headlight, OUTPUT );
        if( pwr_ == 1.0 ) { digitalWrite( _pin_map.Q_left_headlight, 0x1 ); return; }
        analogWrite( _pin_map.Q_left_headlight, this->pwr_2_pwm( pwr_ ) );
    }

public:
    rnk::status_t push_virtual_commander( const virtual_commander_t& vcmd_ ) {
        this->left_headlight( vcmd_.headlight_left );
        this->right_headlight( vcmd_.headlight_right );
        return 0x0;
    }

};


};