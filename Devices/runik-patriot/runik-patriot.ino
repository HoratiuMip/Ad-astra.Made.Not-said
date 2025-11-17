#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#define RNK_KILL_LOGS_LEVEL 0x3
#include "drives/core.hpp"

#include "drives/track.hpp"
#include "drives/lights.hpp"
#include "drives/cli.hpp"
#include "drives/comm.hpp"
#include "drives/hyper.hpp"


rp::Track_drive TrackDrive{ rp::Track_drive::PIN_MAP{
    .Q_right_fwd = 26,
    .Q_right_bck = 27,
    .Q_left_fwd  = 12,
    .Q_left_bck  = 13
} };

rp::Light_drive LightDrive{ rp::Light_drive::PIN_MAP{
    .Q_left_headlight  = 19,
    .Q_right_headlight = 23
} };

rp::Wave_drive WaveDrive{ rp::Wave_drive::PIN_MAP{
    .Q_play  = 15,
    .Q_prev  = 5,
    .Q_next  = 18
} };

rp::Cli_drive CliDrive{
    rp::Cli_drive::PIN_MAP{},
    &TrackDrive,
    &LightDrive
};

rp::Hyper_drive HyperDrive{
    rp::Hyper_drive::PIN_MAP{
        .Q_eye_tx = 33,
        .I_eye_rx = 32
    },
    &TrackDrive,
    &LightDrive,
    &WaveDrive
};

rp::Comm_drive CommDrive{ 
    rp::Comm_drive::PIN_MAP{}, 
    rnk::IO::BLE_UART::Her::PIN_MAP{ Q_light: 2 }, 
    &CliDrive, 
    &HyperDrive 
};


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
    LightDrive.begin();
    WaveDrive.begin();

    WaveDrive.play( 0x1 ); vTaskDelay( 100 ); WaveDrive.play( 0x0 );

    CommDrive.begin( rp::TAG );
    HyperDrive.begin();

    rnk::Log.info( "Boot complete." );
}

void loop( void ) { 
    vTaskDelay( 10000 );
}
