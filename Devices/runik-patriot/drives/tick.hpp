#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"

RP_NAMESPACE {


struct TICKER {
    TICKER() : _ground{ esp_timer_get_time() } {}

    TIME_HR   _ground   = 0;

    RP_inline TIME_HR count( void ) const {
        return esp_timer_get_time() - _ground;
    }
};


struct TIMEOUT {
    TIMEOUT( TIME_HR value ) : _value{ value } {}

    TICKER    _ticker   = {};
    TIME_HR   _value    = 0;

    RP_inline bool triggered( void ) const {
        return _ticker.count() >= _value;
    }

    RP_inline operator bool ( void ) const {
        return this->triggered();
    }
};


};