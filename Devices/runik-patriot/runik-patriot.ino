#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "rnk/Core.hpp"

#include "drives/track.hpp"
#include "drives/cli.hpp"
#include "drives/comm.hpp"

struct rp_virtual_joystick_t {
    float y_l;
    float y_r;
};


rp::Track_drive TrackDrive{ rp::Track_drive::PIN_MAP{
    Q_right_fwd: 26,
    Q_right_bck: 27,
    Q_left_fwd:  12,
    Q_left_bck:  13
} };

rp::Cli_drive CliDrive{
    rp::Cli_drive::PIN_MAP{},
    &TrackDrive
};

rp::Comm_drive CommDrive{ 
    rp::Comm_drive::PIN_MAP{}, 
    rnk::IO::BLE_UART::Her::PIN_MAP{ Q_light: 2 }, 
    &CliDrive, 
    &TrackDrive 
};

// rp::RING_DRIVE RingDrive{ rp::RING_DRIVE::PIN_MAP{
//     i_uart_rx: 17,
//     q_uart_tx: 16,
//     q_reset:   4
// },
//     &Serial2
// };

// rp::BLUE_DRIVE BlueDrive{};



void setup( void ) {
    vTaskDelay( 2000 );
    rnk::status_t status = 0;

    RNK_ASSERT_OR( ( status = rnk::Miru.begin( rnk::miru_begin_args_t{
        tag:      rp::TAG,
        log_port: &Serial,
        log_baud: 115200
    } ) ) == 0x0 ) {
        return;
    };
    rnk::Log.info( "Hello there! Booting..." );

    RNK_ASSERT_OR( ( status = rp::Miru.begin( rp::miru_begin_args_t{
        reset_flash: true
    } ) ) == 0x0 ) {
        rnk::Log.error( "Failed to initialize rp::Miru, [%d].", status );
        return;
    };

    TrackDrive.begin();
    //RingDrive.init();
    //BlueDrive.init( "rp-3000" );

    CommDrive.begin( rp::TAG );


    rnk::Log.info( "Boot complete." );
}

void loop( void ) { 
    vTaskDelay( 100 );

    // if( BlueDrive._port.connected() ) {
    //     auto line = BlueDrive.recv_string( 2'000'000 );

    //     if( !line.empty() ) {
    //         CommandLineInterpreter.exec( line );
    //     }
    // } else {
    //     Serial.println( BlueDrive._port.connect( "3c:8a:1f:a7:92:92" ) );  
    // }

    // if( !Serial.available() ) return;

    // auto [ status, msg ] = CommandLineInterpreter.exec( Serial.readStringUntil( '\n' ).c_str() );
    // Serial.println( msg.data() );
}
