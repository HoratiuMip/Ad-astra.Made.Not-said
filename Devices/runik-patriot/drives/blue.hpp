#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"
#include "tick.hpp"

RP_NAMESPACE {


struct BLUE_DRIVE {
    BluetoothSerial   _port   = {}; 

    STATUS init( const char* access_point_name ) {
        RP_ASSERT_OR( _port.begin( access_point_name ) == true ) return -1;

        return 0;
    }

    std::string recv_string( int timeout_us, bool yield_if_unavailable = true ) {
        std::string acc = {};

        TIMEOUT timeout{ timeout_us };

        for(;;) {
            if( timeout ) return "";

            int c = _port.read();

            if( c == -1 ) { RP_YIELD_CORE; continue; }
            if( c == '\n' ) return acc;

            acc += c;
        }

        return "";
    }

};


};