#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "drives/track-drive.hpp"
#include "drives/ring-drive.hpp"
#include "drives/command-line-interpreter.hpp"


rp::TRACK_DRIVE TrackDrive{ rp::TRACK_DRIVE::PIN_MAP{
    q_right_fwd: 26,
    q_right_bck: 27,
    q_left_fwd:  12,
    q_left_bck:  13
} };

rp::RING_DRIVE RingDrive{ rp::RING_DRIVE::PIN_MAP{
    i_uart_rx: 17,
    q_uart_tx: 16,
    q_reset:   4
},
    &Serial2
};

rp::COMMAND_LINE_INTERPRETER CommandLineInterpreter{
    &TrackDrive, &RingDrive
};


void setup( void ) {
    esp_log_level_set( "*", ESP_LOG_NONE );

    Serial.begin( 19200 ); 
    Serial.println( "[ Runik-Patriot ]: Hello there!" );

    rp::STATUS status = 0;

    RP_SETUP_ASSERT( "rp::Miru", "MIRU", rp::Miru.init( true ) );
    RP_SETUP_ASSERT( "rp::TrackDrive", "TRACK", TrackDrive.init() );
    RP_SETUP_ASSERT( "rp::RingDrive", "RING", RingDrive.init() );
}

void loop( void ) { 
    vTaskDelay( 100 );

    if( !Serial.available() ) return;

    auto [ status, msg ] = CommandLineInterpreter.exec( Serial.readStringUntil( '\n' ).c_str() );
    Serial.println( msg.data() );
}