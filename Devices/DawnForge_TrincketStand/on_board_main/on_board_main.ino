/*====== DawnForge - TrincketStand - Main - Vatca "Mipsan" Tudor-Horatiu 
|
|=== DESCRIPTION
> She's beautiful.
|
======*/
#include <LowPower.h>


typedef   int   GPIO_pin_t;

struct GPIO {
    inline static constexpr int   VCC   = HIGH;
    inline static constexpr int   GND   = LOW;

    inline static constexpr GPIO_pin_t   column_right   = 10;
    inline static constexpr GPIO_pin_t   column_left    = 11;

    inline static constexpr GPIO_pin_t   PIR_sensor   = 2;

    static int init( void ) {
        pinMode( column_right, OUTPUT );
        pinMode( column_left, OUTPUT );

        pinMode( PIR_sensor, INPUT );

        return 0;
    }

    inline static int  D_R( GPIO_pin_t pin )            { return digitalRead( pin ); }
    inline static void D_W( GPIO_pin_t pin, int level ) { digitalWrite( pin, level ); }
    inline static void A_W( GPIO_pin_t pin, int ff )    { analogWrite( pin, ff ); }
};


static struct _Config {
    const long int   WAKE_TIME       = 30e3;
    const long int   WAKE_PWM_T      = 33; 
    const int        PWM_STOP_TH     = 10;

    long int         wake_capture    = 0;
    long int         wake_eff_time   = 0;  
    long int         wake_elapsed    = 0;

} Config;


namespace Routine {


void on_PIR( void ) { 
    Config.wake_eff_time += ( Config.wake_eff_time == 0 ) ? Config.WAKE_TIME : ( Config.wake_elapsed % Config.WAKE_TIME );
}

void sti_PIR( void ) {
    attachInterrupt( digitalPinToInterrupt( GPIO::PIR_sensor ), &on_PIR, RISING );
}

void cli_PIR( void ) {
    detachInterrupt( digitalPinToInterrupt( GPIO::PIR_sensor )  );
}


void wake( void ) {
    bool stop_flag  = false;
    bool stop_left  = false;
    bool stop_right = false;

    for(;;) {
        Config.wake_elapsed = millis() - Config.wake_capture;

        if( Config.wake_elapsed >= Config.wake_eff_time ) stop_flag = true;

        float com_arg   = ( float )Config.wake_elapsed / 1e3f * 3.2f;
        int   left_arg  = ( sin( com_arg ) + 1.0f ) * 127.0f;
        int   right_arg = ( cos( com_arg ) + 1.0f ) * 127.0f;

        if( stop_left || ( stop_flag && left_arg <= Config.PWM_STOP_TH ) ) {
            stop_left = true;
            GPIO::D_W( GPIO::column_left, GPIO::GND );
        } else
            GPIO::A_W( GPIO::column_left, left_arg );

        if( stop_right || ( stop_flag && right_arg <= Config.PWM_STOP_TH ) ) {
            stop_right = true;
            GPIO::D_W( GPIO::column_right, GPIO::GND );
        } else
            GPIO::A_W( GPIO::column_right, right_arg );

        if( stop_left && stop_right ) break;

        delay( Config.WAKE_PWM_T );
    }
}

void sleep( void ) {
    GPIO::D_W( GPIO::column_left, GPIO::GND );
    GPIO::D_W( GPIO::column_right, GPIO::GND );

    Config.wake_eff_time = 0;
    Config.wake_elapsed = 0;

    LowPower.powerDown( SLEEP_FOREVER, ADC_OFF, BOD_OFF );
}


};


void setup( void ) {
    GPIO::init();
    Routine::sti_PIR();
}


void loop( void ) { 
    Routine::sleep();

    Config.wake_capture = millis();
    Routine::wake();
}

