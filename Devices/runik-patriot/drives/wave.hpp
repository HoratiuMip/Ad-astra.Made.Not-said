#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"

namespace rp {


class Wave_drive {
public:
    const struct PIN_MAP {
        rnk::pin_t   Q_play;
        rnk::pin_t   Q_prev;
        rnk::pin_t   Q_next;
    } _pin_map;

public:
    Wave_drive( const PIN_MAP& pin_map_ )
    : _pin_map{ pin_map_ }
    {}

public:
    rnk::status_t begin( void ) {
        pinMode( _pin_map.Q_play, INPUT );
        pinMode( _pin_map.Q_prev, INPUT );
        pinMode( _pin_map.Q_next, INPUT );

        return 0x0;
    }
    
public:
    void play( uint8_t lvl ) {
        if( 0x0 == lvl ) pinMode( _pin_map.Q_play, INPUT );
        else {
            pinMode( _pin_map.Q_play, OUTPUT );
            digitalWrite( _pin_map.Q_play, 0x0 );
        }
    }

    void prev( uint8_t lvl ) {
        if( 0x0 == lvl ) pinMode( _pin_map.Q_prev, INPUT );
        else {
            pinMode( _pin_map.Q_prev, OUTPUT );
            digitalWrite( _pin_map.Q_prev, 0x0 );
        }
    }

    void next( uint8_t lvl ) {
        if( 0x0 == lvl ) pinMode( _pin_map.Q_next, INPUT );
        else {
            pinMode( _pin_map.Q_next, OUTPUT );
            digitalWrite( _pin_map.Q_next, 0x0 );
        }
    }

public:
    rnk::status_t push_virtual_commander( const virtual_commander_t& vcmd_ ) {
        this->play( vcmd_.wave_play );
        this->prev( vcmd_.wave_prev );
        this->next( vcmd_.wave_next );
        return 0x0;
    }

};


};