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
#include "wave.hpp"

namespace rp {


class Hyper_drive {
public:
    const struct PIN_MAP {
        rnk::pin_t   Q_eye_tx;
        rnk::pin_t   I_eye_rx;
    } _pin_map;

public:
    Hyper_drive( const PIN_MAP& pin_map_, Track_drive* track_drive_, Light_drive* light_drive_, Wave_drive* wave_drive_ )
    : _pin_map{ pin_map_ }, _track_drive{ track_drive_ }, _light_drive{ light_drive_ }, _wave_drive{ wave_drive_ }
    {}

RNK_PROTECTED:
    Track_drive*     _track_drive   = NULL;
    Light_drive*     _light_drive   = NULL;
    Wave_drive*      _wave_drive    = NULL;

    HardwareSerial   _eye_uart      = { 0x1 };
    TaskHandle_t     _h_eye_main    = NULL;

    VCMDSource_           _vcmd_src   = VCMDSource_Blue;
    virtual_commander_t   _prev_vcmds[ VCMDSource_COUNT ];

RNK_PROTECTED:
    static void _eye_main( void* arg_ ) {
        Hyper_drive* self = ( Hyper_drive* )arg_;

    for(;;) {
        if( self->_eye_uart.available() ) {
            auto vcmd_http_str = self->_eye_uart.readStringUntil( '\n' );
            char buffer[ 32 ];

            rp::virtual_commander_t vcmd{
                .track_left      = 0.0,
                .track_right     = 0.0,
                .track_pwr       = 1.0,
                .track_mode      = RP_TRACK_MODE_DECOUPLED,
                .headlight_left  = 0.0,
                .headlight_right = 0.0
            };

            RNK_ASSERT_OR( ESP_OK == httpd_query_key_value( vcmd_http_str.c_str(), "tl", buffer, sizeof( buffer ) ) ) continue;
            vcmd.track_left = atof( buffer );
            RNK_ASSERT_OR( ESP_OK == httpd_query_key_value( vcmd_http_str.c_str(), "tr", buffer, sizeof( buffer ) ) ) continue;
            vcmd.track_right = atof( buffer );
            RNK_ASSERT_OR( ESP_OK == httpd_query_key_value( vcmd_http_str.c_str(), "tp", buffer, sizeof( buffer ) ) ) continue;
            vcmd.track_pwr = atof( buffer );
            RNK_ASSERT_OR( ESP_OK == httpd_query_key_value( vcmd_http_str.c_str(), "tm", buffer, sizeof( buffer ) ) ) continue;
            vcmd.track_mode = atoi( buffer );
            RNK_ASSERT_OR( ESP_OK == httpd_query_key_value( vcmd_http_str.c_str(), "hl", buffer, sizeof( buffer ) ) ) continue;
            vcmd.headlight_left = vcmd.headlight_right = atof( buffer );

            self->push_virtual_commander( vcmd, VCMDSource_Eye );
        } else {
            vTaskDelay( 50 );
        }
    } }

public:
    rnk::status_t begin( void ) {
        _eye_uart.begin( 115200, SERIAL_8N1, _pin_map.I_eye_rx, _pin_map.Q_eye_tx );
        xTaskCreate( &Hyper_drive::_eye_main, "Hyper_drive::_eye_main", 4096, ( void* )this, rnk::TaskPriority_Urgent, &_h_eye_main ); 
        return 0x0;
    }
    
public:
    rnk::status_t push_virtual_commander( const virtual_commander_t& vcmd_, VCMDSource_ src_ ) {
        auto& prev_vcmd = _prev_vcmds[ src_ ];
        if( src_ != _vcmd_src && vcmd_ != prev_vcmd ) {
            prev_vcmd = vcmd_;
            _vcmd_src = src_;
        } else if( src_ == _vcmd_src ) {
            return _track_drive->push_virtual_commander( vcmd_ ) | _light_drive->push_virtual_commander( vcmd_ ) | _wave_drive->push_virtual_commander( vcmd_ );
        }
        return 0x0;
    }

};


};