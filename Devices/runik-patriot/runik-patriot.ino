#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "drives/track.hpp"
#include "drives/ring.hpp"
#include "drives/blue.hpp"
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

rp::BLUE_DRIVE BlueDrive{};

rp::COMMAND_LINE_INTERPRETER CommandLineInterpreter{
    &TrackDrive, &RingDrive
};


void setup( void ) {
    rp::STATUS status = 0;

    rp::Echo.init();
    rp::Echo.inf( "Booting..." );

    RP_ASSERT_OR( rp::Miru.init( rp::MIRU::INIT_ARGS{
        reset_flash: true
    } ) == 0 ) {
        rp::Echo.err( "Master init error." );    
        return;
    };

    TrackDrive.init();
    //RingDrive.init();
    BlueDrive.init( "rp-3000" );

    rp::Echo.inf( "Boot ok." );
}

void loop( void ) { 
    vTaskDelay( 100 );

    if( BlueDrive._port.connected() ) {
        auto line = BlueDrive.recv_string( 2'000'000 );

        if( !line.empty() ) {
            CommandLineInterpreter.exec( line );
        }
    }

    if( !Serial.available() ) return;

    auto [ status, msg ] = CommandLineInterpreter.exec( Serial.readStringUntil( '\n' ).c_str() );
    Serial.println( msg.data() );
}
