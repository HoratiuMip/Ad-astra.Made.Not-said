#define BARRACUDA_CTRL_BUILD_FOR_ON_BOARD
#define BARRACUDA_CTRL_ARCHITECTURE_LITTLE
#include "../../../IXT/Include/IXT/SpecMod/barracuda-ctrl.hpp"

#include "BluetoothSerial.h"

#include <MPU6050_WE.h>
#include <Wire.h>


enum LOG_LEVEL {
  LOG_OK, LOG_WARNING, LOG_ERROR, LOG_CRITICAL, LOG_INFO
};
const char* LOG_LEVEL_STR[] = { "OK - ", "WARNING - ", "ERROR - ", "CRITICAL - ", "INFO - " };
struct _SERIAL_LOG {
  _SERIAL_LOG& operator () ( LOG_LEVEL ll = LOG_INFO ) {
    Serial.print( LOG_LEVEL_STR[ ll ] );
    return *this;
  }

  template< typename T >
  _SERIAL_LOG& operator << ( const T& frag ) {
    Serial.print( frag );
    return *this;
  }

} SERIAL_LOG;


typedef   const int8_t   GPIO_pin_t;
struct {
  struct { GPIO_pin_t sw, x, y; } rachel{ sw: 34, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
  /* JOYSTICKS                    |lower left                     |upper right */

  GPIO_pin_t giselle = 5, karina = 18, ningning = 19, winter = 23;
  /* SWS     |blue        |red         |yellow        |green */

  struct { GPIO_pin_t r, g, b; } bitna{ r: 13, g: 12, b: 14 };

  int init() {
    SERIAL_LOG() << "Configuring pin modes... ";

    pinMode( rachel.sw, INPUT_PULLUP ); pinMode( rachel.x, INPUT ); pinMode( rachel.y, INPUT );
    pinMode( samantha.sw, INPUT_PULLUP ); pinMode( samantha.x, INPUT ); pinMode( samantha.y, INPUT );

    pinMode( giselle,  INPUT_PULLUP );
    pinMode( karina,   INPUT_PULLUP );
    pinMode( ningning, INPUT_PULLUP );
    pinMode( winter,   INPUT_PULLUP );

    pinMode( bitna.r, OUTPUT ); pinMode( bitna.g, OUTPUT ); pinMode( bitna.b, OUTPUT );

    SERIAL_LOG << "ok.\n";

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
  BluetoothSerial   blue;

  const int32_t     mpu_addr   = 0x68;
  MPU6050_WE        mpu        = MPU6050_WE( mpu_addr );

  int init() {
    SERIAL_LOG() << "Bluetooth serial begin... ";
    blue.begin( barracuda_ctrl::DEVICE_NAME );
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "I^2C wire begin... ";
    Wire.begin();
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "MPU66050 init... ";
    if( !mpu.init() ) {
      SERIAL_LOG << "fault.\n";
      return -1;
    }
    else {
      SERIAL_LOG << "ok.\n";
    }
    
    return 0;
  }

} COM;


struct _PROTO : barracuda_ctrl::out_cache_t< 128 > {
  int init( void ) {
    return 0;
  }

  int _out_cache_write( int sz ) override {
    COM.blue.write( _out_cache, sizeof( *_out_cache_head ) + sz );

    return 0;
  }

  int resolve_inbound_head( void ) {
    if( COM.blue.peek() < 0 ) return 0;

    barracuda_ctrl::proto_head_t in_head;
    int idx = 0;

    do {
      ( ( uint8_t* )&in_head )[ idx ] = ( uint8_t )COM.blue.read();
    } while( ++idx < sizeof( in_head ) );

    if( ( in_head.sig & barracuda_ctrl::PROTO_SIG_MSK ) != barracuda_ctrl::PROTO_SIG ) {
      SERIAL_LOG( LOG_ERROR ) << "Incoming byte stream is out of alignment.\n";
      return -1;
    }

    switch( in_head._dw0.op ) {
      case barracuda_ctrl::PROTO_OP_PING: {
        _out_cache_head->_dw0.op  = barracuda_ctrl::PROTO_OP_ACK;
        _out_cache_head->_dw1.seq = in_head._dw1.seq;
        _out_cache_head->_dw2.sz  = 0;

        SERIAL_LOG() << "Responding to ping on sequence ( " << in_head._dw1.seq << " )... ";
        this->_out_cache_write( 0 );
        SERIAL_LOG << " ok.\n";
      break; }
    }

    return 0;
  }

} PROTO;


typedef   int8_t   LED_RGB;
struct LED {
  inline static constexpr LED_RGB rgbs[ 8 ] = { 0b000, 0b100, 0b110, 0b010, 0b011, 0b001, 0b101, 0b111 };
  enum IDX { BLK, RED, YLW, GRN, TRQ, BLU, PRP, WHT };

  LED_RGB crt;

  int set( LED_RGB rgb ) {
    digitalWrite( GPIO.bitna.r, ( rgb >> 2 ) & 1 );
    digitalWrite( GPIO.bitna.g, ( rgb >> 1 ) & 1 );
    digitalWrite( GPIO.bitna.b, ( rgb ) & 1 );
    crt = rgb;
    return 0;
  }
  int set( IDX idx ) { return this->set( rgbs[ idx ] ); }

  int operator () ( int8_t idx ) { return this->set( rgbs[ idx ] ); }

  int blink( LED_RGB rgb, bool keep_state, int N, int ms_on, int ms_off ) {
    for( int n = 1; n <= N; ++n ) {
      this->set( rgb ); delay( ms_on ); this->set( BLK ); delay( ms_off );
    }
    if( keep_state ) this->set( rgb );
    return 0;
  }
  template< typename ...Args >
  int blink( IDX idx, const Args&... args ) { return this->blink( rgbs[ idx ], args... ); } 

  void test_rgb( int ms ) {
    LED_RGB prev = crt;

    for( int8_t rgb : rgbs ) {
      this->set( rgb ); delay( ms );
    }

    this->set( prev );
  }

} BITNA;


struct _DYNAMIC : barracuda_ctrl::proto_head_t, barracuda_ctrl::dynamic_state_t {
  struct {
    struct { float x, y; } rachel, samantha;
  } _idle_reads;

  int init( void ) {
    SERIAL_LOG() << "Proto head init... ";
    barracuda_ctrl::proto_head_t::_dw0.op = barracuda_ctrl::PROTO_OP_DYNAMIC;
    barracuda_ctrl::proto_head_t::_dw2.sz = sizeof( barracuda_ctrl::dynamic_state_t ); 
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "Joysticks calibrate... ";
    _idle_reads.rachel.x = analog_read_mean< float >( GPIO.rachel.x, 10, 1 ); _idle_reads.rachel.y = analog_read_mean< float >( GPIO.rachel.y, 10, 1 );
    _idle_reads.samantha.x = analog_read_mean< float >( GPIO.samantha.x, 10, 1 ); _idle_reads.samantha.y = analog_read_mean< float >( GPIO.samantha.y, 10, 1 );
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "MPU6050 calibrate... ";
    COM.mpu.autoOffsets();
    COM.mpu.setGyrRange( MPU6050_GYRO_RANGE_250 );
    COM.mpu.setAccRange( MPU6050_ACC_RANGE_2G );
    SERIAL_LOG << "ok.\n";

    return 0;
  }

  void scan( void ) {
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

  void blue_tx_dynamic_state( void ) {
    this->acquire_seq();
    COM.blue.write( ( uint8_t* )this, sizeof( barracuda_ctrl::proto_head_t ) + barracuda_ctrl::proto_head_t::_dw2.sz );
  }

  void serial_tx_dynamic_state( void ) {
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


#define SETUP_CRITICAL_ASSERT( c ) if( !c ) { SERIAL_LOG( LOG_CRITICAL ) << "CRITICAL ASSERT ( #c ) FAILED. ENTERING UNRECOVARABLE STATE.\n"; goto l_unrecovarable_fault; }
void setup( void ) {
{
  Serial.begin( 115200 );

  SETUP_CRITICAL_ASSERT( GPIO.init() == 0 );
  
  BITNA.blink( LED::RED, true, 16, 25, 50 );

  SETUP_CRITICAL_ASSERT( COM.init() == 0 );

  SETUP_CRITICAL_ASSERT( DYNAMIC.init() == 0 );

  SERIAL_LOG( LOG_OK ) << "Setup complete.\n";
  BITNA.blink( LED::GRN, true, 16, 25, 50 );

  BITNA.blink( LED::TRQ, true, 12, 160, 160 );
  DYNAMIC.scan();
  if( DYNAMIC.giselle.dwn ) {
    BITNA.blink( LED::TRQ, true, 16, 50, 50 );
    BITNA.test_rgb( 2000 );
    BITNA.blink( LED::TRQ, true, 16, 50, 50 );
  }

  BITNA( LED::BLU );
} return;

l_unrecovarable_fault: {
  while( 1 ) BITNA.blink( LED::RED, false, 1, 2000, 1000 );
} return;
}

void loop( void ) {
    if( COM.blue.connected() ) {
      PROTO.resolve_inbound_head();
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