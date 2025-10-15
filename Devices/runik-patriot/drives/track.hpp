#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"

RP_NAMESPACE {


struct TRACK_DRIVE {
    const struct PIN_MAP {
        PIN   q_right_fwd;
        PIN   q_right_bck;
        PIN   q_left_fwd;
        PIN   q_left_bck;
    } _pin_map;

    inline static constexpr int   PWM_TOP   = 255;

    TRACK_DRIVE( const PIN_MAP& pin_map )
    : _pin_map{ pin_map }
    {}

    STATUS init( void ) {
        pinMode( _pin_map.q_right_fwd, OUTPUT );
        pinMode( _pin_map.q_right_bck, OUTPUT );
        pinMode( _pin_map.q_left_fwd,  OUTPUT );
        pinMode( _pin_map.q_left_bck,  OUTPUT );
        return 0;
    }
    
    RP_inline int pwr_2_pwm( float pwr ) {
        return pwr * PWM_TOP;
    }


    RP_inline STATUS right_halt( void ) {
        analogWrite( _pin_map.q_right_fwd, 0x0 );
        analogWrite( _pin_map.q_right_bck, 0x0 );
        return 0;
    }

    RP_inline STATUS left_halt( void ) {
        analogWrite( _pin_map.q_left_fwd, 0x0 );
        analogWrite( _pin_map.q_left_bck, 0x0 );
        return 0;
    }

    RP_inline STATUS halt( void ) {
        return this->right_halt() | this->left_halt();
    }


    RP_inline STATUS right_fwd_raw( PWM pwm ) {
        analogWrite( _pin_map.q_right_bck, 0x0 );
        analogWrite( _pin_map.q_right_fwd, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    RP_inline STATUS right_bck_raw( PWM pwm ) {
        analogWrite( _pin_map.q_right_fwd, 0x0 );
        analogWrite( _pin_map.q_right_bck, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    RP_inline STATUS left_fwd_raw( PWM pwm ) {
        analogWrite( _pin_map.q_left_bck, 0x0 );
        analogWrite( _pin_map.q_left_fwd, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    RP_inline STATUS left_bck_raw( PWM pwm ) {
        analogWrite( _pin_map.q_left_fwd, 0x0 );
        analogWrite( _pin_map.q_left_bck, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    
    RP_inline STATUS right_fwd( float pwr ) {
        return this->right_fwd_raw( this->pwr_2_pwm( pwr ) );
    }

    RP_inline STATUS right_bck( float pwr ) {
        return this->right_bck_raw( this->pwr_2_pwm( pwr ) );
    }

    RP_inline STATUS left_fwd( float pwr ) {
        return this->left_fwd_raw( this->pwr_2_pwm( pwr ) );
    }

    RP_inline STATUS left_bck( float pwr ) {
        return this->left_bck_raw( this->pwr_2_pwm( pwr ) );
    }


    STATUS seq_1( float pwr, int ms ) {
        this->halt();
        this->right_fwd( pwr ); vTaskDelay( ms ); this->right_halt();
        this->right_bck( pwr ); vTaskDelay( ms ); this->right_halt();
        this->left_fwd( pwr ); vTaskDelay( ms ); this->left_halt();
        this->left_bck( pwr ); vTaskDelay( ms ); this->left_halt();
        return 0;
    }

    STATUS seq_2( float pwr, int ms ) {
        this->halt();
        this->right_fwd( pwr ); this->left_bck( pwr );
        vTaskDelay( ms );
        this->halt();
    }

    STATUS seq_3( float pwr, int ms ) {
        this->halt();
        this->left_fwd( pwr ); this->right_bck( pwr );
        vTaskDelay( ms );
        this->halt();
    }

    STATUS seq_4( float pwr, int ms ) {
        this->halt();
        this->left_fwd( pwr ); this->right_fwd( pwr );
        vTaskDelay( ms );
        this->halt();
    }

    STATUS seq_5( float pwr, int ms ) {
        this->halt();
        this->left_bck( pwr ); this->right_bck( pwr );
        vTaskDelay( ms );
        this->halt();
    }

};


};