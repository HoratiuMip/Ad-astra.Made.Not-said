#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"
#include "track.hpp"
#include "lights.hpp"

namespace rp {


class Hyper_drive {
public:
    const struct PIN_MAP {
    } _pin_map;

public:
    Hyper_drive( const PIN_MAP& pin_map_, Track_drive* track_drive_, Light_drive* light_drive_ )
    : _pin_map{ pin_map_ }, _track_drive{ track_drive_ }, _light_drive{ light_drive_ }
    {}

RNK_PROTECTED:
    Track_drive*   _track_drive   = NULL;
    Light_drive*   _light_drive   = NULL;

public:
    rnk::status_t begin( void ) {
        return 0x0;
    }
    
public:
    rnk::status_t push_virtual_commander( const virtual_commander_t& vcmd_ ) {
        return _track_drive->push_virtual_commander( vcmd_ ) | _light_drive->push_virtual_commander( vcmd_ );
    }

};


};