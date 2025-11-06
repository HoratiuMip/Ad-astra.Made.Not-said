#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"

namespace rp {


class Track_drive {
public:
    const struct PIN_MAP {
        rnk::pin_t   Q_right_fwd;
        rnk::pin_t   Q_right_bck;
        rnk::pin_t   Q_left_fwd;
        rnk::pin_t   Q_left_bck;
    } _pin_map;

public:
    inline static constexpr int   PWM_TOP   = 255;

RNK_PROTECTED:
    TimerHandle_t   _h_vcmd_timer   = NULL;

public:
    Track_drive( const PIN_MAP& pin_map )
    : _pin_map{ pin_map }
    {}

public:
    rnk::status_t begin( void ) {
        pinMode( _pin_map.Q_right_fwd, OUTPUT );
        pinMode( _pin_map.Q_right_bck, OUTPUT );
        pinMode( _pin_map.Q_left_fwd,  OUTPUT );
        pinMode( _pin_map.Q_left_bck,  OUTPUT );

        _h_vcmd_timer = xTimerCreate(
            "rp::Track_drive::_vcmd_timer",
            pdMS_TO_TICKS( 200 ), 
            false,
            ( void* )this,  
            &Track_drive::_vcmd_timer 
        );
        xTimerStart( _h_vcmd_timer, 0 );

        return 0x0;
    }
    
    RNK_inline int pwr_2_pwm( float pwr ) {
        return pwr * PWM_TOP;
    }

RNK_PROTECTED:
    static void _vcmd_timer( TimerHandle_t h_vcmd_timer_ ) {
        ( ( Track_drive* )pvTimerGetTimerID( h_vcmd_timer_ ) )->halt();
    }

public:
    RNK_inline rnk::status_t right_halt( void ) {
        analogWrite( _pin_map.Q_right_fwd, 0x0 );
        analogWrite( _pin_map.Q_right_bck, 0x0 );
        return 0;
    }

    RNK_inline rnk::status_t left_halt( void ) {
        analogWrite( _pin_map.Q_left_fwd, 0x0 );
        analogWrite( _pin_map.Q_left_bck, 0x0 );
        return 0;
    }

    RNK_inline rnk::status_t halt( void ) {
        return this->right_halt() | this->left_halt();
    }

public:
    RNK_inline rnk::status_t right_fwd_raw( rnk::pwm_t pwm ) {
        analogWrite( _pin_map.Q_right_bck, 0x0 );
        analogWrite( _pin_map.Q_right_fwd, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    RNK_inline rnk::status_t right_bck_raw( rnk::pwm_t pwm ) {
        analogWrite( _pin_map.Q_right_fwd, 0x0 );
        analogWrite( _pin_map.Q_right_bck, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    RNK_inline rnk::status_t left_fwd_raw( rnk::pwm_t pwm ) {
        analogWrite( _pin_map.Q_left_bck, 0x0 );
        analogWrite( _pin_map.Q_left_fwd, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

    RNK_inline rnk::status_t left_bck_raw( rnk::pwm_t pwm ) {
        analogWrite( _pin_map.Q_left_fwd, 0x0 );
        analogWrite( _pin_map.Q_left_bck, min( pwm, Miru.FLASH.Track.PWM_LIMIT ) );
        return 0;
    }

public: 
    RNK_inline rnk::status_t right_fwd( float pwr ) {
        return this->right_fwd_raw( this->pwr_2_pwm( pwr ) );
    }

    RNK_inline rnk::status_t right_bck( float pwr ) {
        return this->right_bck_raw( this->pwr_2_pwm( pwr ) );
    }

    RNK_inline rnk::status_t left_fwd( float pwr ) {
        return this->left_fwd_raw( this->pwr_2_pwm( pwr ) );
    }

    RNK_inline rnk::status_t left_bck( float pwr ) {
        return this->left_bck_raw( this->pwr_2_pwm( pwr ) );
    }

public:
    RNK_inline void right( float pwr_, float threshold_ = 0.05 ) {
        if( pwr_ > threshold_ ) this->right_fwd( pwr_ );
        else if( pwr_ < -threshold_ ) this->right_bck( -pwr_ );
        else this->right_halt(); 
    }

    RNK_inline void left( float pwr_, float threshold_ = 0.05 ) {
        if( pwr_ > threshold_ ) this->left_fwd( pwr_ );
        else if( pwr_ < -threshold_ ) this->left_bck( -pwr_ );
        else this->left_halt(); 
    }

public:
    rnk::status_t seq_1( float pwr, int ms ) {
        this->halt();
        this->right_fwd( pwr ); vTaskDelay( ms ); this->right_halt();
        this->right_bck( pwr ); vTaskDelay( ms ); this->right_halt();
        this->left_fwd( pwr ); vTaskDelay( ms ); this->left_halt();
        this->left_bck( pwr ); vTaskDelay( ms ); this->left_halt();
        return 0;
    }

    rnk::status_t seq_2( float pwr, int ms ) {
        this->halt();
        this->right_fwd( pwr ); this->left_bck( pwr );
        vTaskDelay( ms );
        this->halt();
    }

    rnk::status_t seq_3( float pwr, int ms ) {
        this->halt();
        this->left_fwd( pwr ); this->right_bck( pwr );
        vTaskDelay( ms );
        this->halt();
    }

    rnk::status_t seq_4( float pwr, int ms ) {
        this->halt();
        this->left_fwd( pwr ); this->right_fwd( pwr );
        vTaskDelay( ms );
        this->halt();
    }

    rnk::status_t seq_5( float pwr, int ms ) {
        this->halt();
        this->left_bck( pwr ); this->right_bck( pwr );
        vTaskDelay( ms );
        this->halt();
    }
public:
    rnk::status_t push_virtual_commander( const virtual_commander_t& vcmd_ ) {
        xTimerReset( _h_vcmd_timer, 0 );
        switch( vcmd_.track_mode ) {
            case RP_TRACK_MODE_DECOUPLED: {
                this->right( vcmd_.track_right * vcmd_.track_pwr );
                this->left( vcmd_.track_left * vcmd_.track_pwr );
            break; }

            case RP_TRACK_MODE_DIFFERENTIAL: {
                const uint32_t sgn_msk = ( *( uint32_t* )&vcmd_.track_right ) & 0x80000000;
                float diff = vcmd_.track_right * vcmd_.track_right;
                ( *( uint32_t* )&diff ) |= sgn_msk;
                this->left( ( vcmd_.track_left + diff ) * vcmd_.track_pwr );
                this->right( ( vcmd_.track_left - diff ) * vcmd_.track_pwr );
            break; }

            default: this->halt();
        }
       
        return 0x0;
    }
};


};