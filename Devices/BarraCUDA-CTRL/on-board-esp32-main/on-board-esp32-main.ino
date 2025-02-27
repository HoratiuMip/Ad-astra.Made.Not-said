#define BARRACUDA_CTRL_BUILD_FOR_ON_BOARD
#define BARRACUDA_CTRL_ARCHITECTURE_LITTLE
#include "../../../IXT/Include/IXT/SpecMod/barracuda-ctrl.hpp"

#include "BluetoothSerial.h"

#include <MPU6050_WE.h>
#include <Wire.h>


#define LOG_PREFIX "BarraCUDA-CTRL: "
#define SERIAL_LOG( msg ) Serial.print( LOG_PREFIX ),Serial.print( msg )
#define SERIAL_LOGL( msg ) Serial.print( LOG_PREFIX ),Serial.println( msg )


typedef   const int8_t   GPIO_pin_t;
struct {
  struct { GPIO_pin_t sw, x, y; } rachel{ sw: 34, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
  /* JOYSTICKS                    |lower left                     |upper right */

  GPIO_pin_t giselle = 5, karina = 18, ningning = 19, winter = 23;
  /* SWS     |blue        |red         |yellow        |green */

  int init() {
    SERIAL_LOGL( "Configuring pin modes..." );

    pinMode( rachel.sw, INPUT_PULLUP ); pinMode( rachel.x, INPUT ); pinMode( rachel.y, INPUT );

    pinMode( samantha.sw, INPUT_PULLUP ); pinMode( samantha.x, INPUT ); pinMode( samantha.y, INPUT );

    pinMode( giselle,  INPUT_PULLUP );
    pinMode( karina,   INPUT_PULLUP );
    pinMode( ningning, INPUT_PULLUP );
    pinMode( winter,   INPUT_PULLUP );

    return 0;
  }

} GPIO;


template< typename T >
T analog_read_mean( GPIO_pin_t pin, int reads, int dt_ms ) {
  int acc = 0;

  for( int n = 1; n <= reads; ++n ) {
    acc += analogRead( pin );
    if( dt_ms > 0 ) delay( dt_ms );
  }

  return ( T )acc / reads;
}


struct {
  BluetoothSerial   blue_serial;

  const int32_t     mpu_addr   = 0x68;
  MPU6050_WE        mpu        = MPU6050_WE( mpu_addr );

  int init() {
    SERIAL_LOGL( "Beginning bluetooth serial..." );
    blue_serial.begin( barracuda_ctrl::DEVICE_NAME );

    if( !mpu.init() ) {
      SERIAL_LOGL( "MPU6050 initialization failure." );
      return -1;
    }
    else {
      SERIAL_LOGL( "MPU6050 initialization ok." );
    }
    
    return 0;
  }

} COM;


struct Dynamic : barracuda_ctrl::proto_head_t, barracuda_ctrl::dynamic_state_t {
  struct {
    struct { float x, y; } rachel, samantha;
  } _idle_reads;

  void init() {
    SERIAL_LOGL( "Preparing protocol header..." );
    barracuda_ctrl::proto_head_t::_dw0.op = barracuda_ctrl::PROTO_OP_CODE_DESC;
    barracuda_ctrl::proto_head_t::_dw1    = sizeof( barracuda_ctrl::dynamic_state_t ); 

    SERIAL_LOGL( "Calibrating joysticks..." );
    _idle_reads.rachel.x = analog_read_mean< float >( GPIO.rachel.x, 10, 1 ); _idle_reads.rachel.y = analog_read_mean< float >( GPIO.rachel.y, 10, 1 );
    _idle_reads.samantha.x = analog_read_mean< float >( GPIO.samantha.x, 10, 1 ); _idle_reads.samantha.y = analog_read_mean< float >( GPIO.samantha.y, 10, 1 );

    SERIAL_LOGL( "Calibrating MPU..." );
    COM.mpu.autoOffsets();
    COM.mpu.setGyrRange( MPU6050_GYRO_RANGE_250 );
    COM.mpu.setAccRange( MPU6050_ACC_RANGE_2G );
  }

  void scan() {
    rachel.x = analogRead( GPIO.rachel.x ); rachel.y = analogRead( GPIO.rachel.y );
    samantha.x = analogRead( GPIO.samantha.x ); samantha.y = analogRead( GPIO.samantha.y );

    const auto _resolve_joystick_axis = [] ( float& read, float idle_read ) -> void {
      if( ( read -= idle_read ) < .0 ) 
        read /= idle_read;
      else
        read /= 4095.0 - idle_read;
    };

    _resolve_joystick_axis( rachel.x, _idle_reads.rachel.x ); _resolve_joystick_axis( rachel.y, _idle_reads.rachel.y );
    _resolve_joystick_axis( samantha.x, _idle_reads.samantha.x ); _resolve_joystick_axis( samantha.y, _idle_reads.samantha.y );

    samantha.y *= -1.0;
    

    const auto _resolve_switch = [] ( barracuda_ctrl::switch_t& sw, GPIO_pin_t pin ) -> void {
      int is_dwn = !digitalRead( pin );

      switch( ( sw.dwn << 1 ) | is_dwn ) {
        case 0: [[fallthrough]];
        case 3: sw.rls = 0; sw.prs = 0; break;
        case 1: sw.rls = 0; sw.prs = 1; break;
        case 2: sw.rls = 1; sw.prs = 0; break;
      }

      sw.dwn = is_dwn;
    };

    _resolve_switch( rachel.sw,   GPIO.rachel.sw );
    _resolve_switch( samantha.sw, GPIO.samantha.sw );
    _resolve_switch( giselle,     GPIO.giselle );
    _resolve_switch( karina,      GPIO.karina );
    _resolve_switch( ningning,    GPIO.ningning );
    _resolve_switch( winter,      GPIO.winter);
  }

  void blue_tx_dynamic_state() {
    COM.blue_serial.write( ( uint8_t* )this, sizeof( barracuda_ctrl::proto_head_t ) + barracuda_ctrl::proto_head_t::_dw1 );
  }

  void serial_tx_dynamic_state() {
    char buffer[ 256 ];

    sprintf( buffer, 
        "Rachel( S, X, Y ) = ( %d, %f, %f )\n" \
        "Samantha( S, X, Y ) = ( %d, %f, %f )\n" \
        "Switches( G, Y, R, B ) = ( %d, %d, %d, %d )\0",
        rachel.sw.dwn, rachel.x, rachel.y,
        samantha.sw.dwn, samantha.x, samantha.y,
        karina.dwn, karina.dwn, karina.dwn, karina.dwn
    );

    Serial.println( buffer );
  }
} DYNAMIC;


void setup() {
    Serial.begin( 115200 );

    GPIO.init();
    COM.init();
    DYNAMIC.init();
    
    Wire.begin();

    SERIAL_LOGL( "Setup ok." );
}

void loop() {
    if ( COM.blue_serial.available() ) {
        DYNAMIC.scan();
        DYNAMIC.blue_tx_dynamic_state();
        //DYNAMIC.print_to_serial();

        delay(20);
    }

    xyzFloat gValue = COM.mpu.getGValues();
  xyzFloat gyr = COM.mpu.getGyrValues();
  float temp = COM.mpu.getTemperature();
  float resultantG = COM.mpu.getResultantG(gValue);

  // Serial.println("Acceleration in g (x,y,z):");
  // Serial.print(gValue.x);
  // Serial.print("   ");
  // Serial.print(gValue.y);
  // Serial.print("   ");
  // Serial.println(gValue.z);
  // Serial.print("Resultant g: ");
  // Serial.println(resultantG);

  // Serial.println("Gyroscope data in degrees/s: ");
  // Serial.print(gyr.x);
  // Serial.print("   ");
  // Serial.print(gyr.y);
  // Serial.print("   ");
  // Serial.println(gyr.z);

  // Serial.print("Integrated sensor temperature read: ");
  // Serial.print(temp);
  // Serial.println( "°C" );

}
