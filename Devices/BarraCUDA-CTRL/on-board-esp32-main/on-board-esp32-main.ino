#include "BluetoothSerial.h"

#include <MPU6050_WE.h>
#include <Wire.h>

#include "../barracuda-ctrl.hpp"
#define BAR_PROTO_ARCHITECTURE_LITTLE
#include "../bar-proto.hpp"

#include <atomic>
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


typedef   int8_t   GPIO_pin_t;
typedef   int8_t   LED_RGB;


struct _PARAMS {
  void reset() {
    SERIAL_LOG( LOG_INFO ) << "Resetting parameters... ";
    *this = _PARAMS{};
    SERIAL_LOG << "ok.\n";
  }

  bool      _conn_rst         = true;
  int32_t   _bar_proto_seq    = 0;

  int32_t   main_loop_delay   = 20;

} PARAMS;


struct { 
  struct { const GPIO_pin_t sw, x, y; } rachel{ sw: 27, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
  /*       JOYSTICKS                    |lower left                     |upper right */

  const GPIO_pin_t giselle = 5, karina = 18, ningning = 19, winter = 23;
  /*      SWS     |blue        |red         |yellow        |green */

  struct { const GPIO_pin_t r, g, b; } bitna{ r: 13, g: 12, b: 14 };

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

  int init() {
    SERIAL_LOG() << "Bluetooth serial begin... ";
    blue.begin( bar_ctrl::DEVICE_NAME );
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "I^2C wire begin... ";
    Wire.begin();
    SERIAL_LOG << "ok.\n";
    
    return 0;
  }

  int blue_itr_recv( void* ptr, int sz ) {
    int count = 0;
   
    do {
      ( ( uint8_t* )ptr )[ count ] = ( uint8_t )blue.read();
    } while( ++count < sz );

    return count;
  }

  int blue_itr_send( void* ptr, int sz ) {
    int count = 0;
   
    do {
      int ret = blue.write( ( uint8_t* )ptr + count, sz - count );
      count += ret;
    } while( count < sz );

    return count;
  }

} COM;


struct _GRAN : public MPU6050_WE {
  int init( void ) {
    SERIAL_LOG() << "MPU6050 init... ";
    if( !this->MPU6050_WE::init() ) {
      SERIAL_LOG << "fault.\n";
      return -1;
    }
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "MPU6050 calibrate... ";
    this->MPU6050_WE::autoOffsets();
    this->MPU6050_WE::setAccRange( _acc_range );
    this->MPU6050_WE::setGyrRange( _gyr_range );
    SERIAL_LOG << "ok.\n";

    return 0;
  }

  bool set_acc_range( MPU9250_accRange range ) {
    if( range < MPU6050_ACC_RANGE_2G || range > MPU6050_ACC_RANGE_16G ) return false;
    this->MPU6050_WE::setAccRange( _acc_range = range );
    return true;
  }

  bool set_gyr_range( MPU9250_gyroRange range ) {
    if( range < MPU6050_GYRO_RANGE_250 || range > MPU6050_GYRO_RANGE_2000 ) return false;
    this->MPU6050_WE::setGyrRange( _gyr_range = range );
    return true;
  }

  MPU9250_accRange    _acc_range   = MPU6050_ACC_RANGE_2G;          
  MPU9250_gyroRange   _gyr_range   = MPU6050_GYRO_RANGE_250;  

} GRAN{ MPU6050_WE::WHO_AM_I_CODE };


struct LED {
  inline static constexpr LED_RGB rgbs[ 8 ] = { 0b000, 0b100, 0b110, 0b010, 0b011, 0b001, 0b101, 0b111 };
  enum IDX { BLK, RED, YLW, GRN, TRQ, BLU, PRP, WHT };

  LED_RGB _crt;

  int set( LED_RGB rgb ) {
    digitalWrite( GPIO.bitna.r, ( rgb >> 2 ) & 1 );
    digitalWrite( GPIO.bitna.g, ( rgb >> 1 ) & 1 );
    digitalWrite( GPIO.bitna.b, ( rgb ) & 1 );
    _crt = rgb;
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
    LED_RGB prev = _crt;

    for( int8_t rgb : rgbs ) {
      this->set( rgb ); delay( ms );
    }

    this->set( prev );
  }

} BITNA;


struct _DYNAMIC : bar_proto_head_t, bar_ctrl::dynamic_state_t {
  struct {
    struct { float x, y; } rachel, samantha;
  } _idle_reads;

  int init( void ) {
    SERIAL_LOG() << "Proto head init... ";
    bar_proto_head_t::_dw0.op = BAR_PROTO_OP_BURST;
    bar_proto_head_t::_dw2.sz = sizeof( bar_ctrl::dynamic_state_t ); 
    SERIAL_LOG << "ok.\n";

    SERIAL_LOG() << "Joysticks calibrate... ";
    _idle_reads.rachel.x = analog_read_mean< float >( GPIO.rachel.x, 10, 1 ); _idle_reads.rachel.y = analog_read_mean< float >( GPIO.rachel.y, 10, 1 );
    _idle_reads.samantha.x = analog_read_mean< float >( GPIO.samantha.x, 10, 1 ); _idle_reads.samantha.y = analog_read_mean< float >( GPIO.samantha.y, 10, 1 );
    SERIAL_LOG << "ok.\n";

    return GRAN.init();
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
    

    const auto _resolve_switch = [] ( bar_ctrl::switch_t& sw, GPIO_pin_t pin ) -> void {
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


    xyzFloat acc_read = GRAN.getGValues();
    gran.acc = { x: acc_read.y, y: -acc_read.x, z: acc_read.z };
    xyzFloat gyr_read = GRAN.getGyrValues();
    gran.gyr = { x: gyr_read.y, y: -gyr_read.x, z: gyr_read.z };
  }

  void blue_tx( void ) {
    bar_proto_head_t::_dw1.seq = PARAMS._bar_proto_seq++;
    COM.blue.write( ( uint8_t* )this, sizeof( bar_proto_head_t ) + bar_proto_head_t::_dw2.sz );
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


BAR_PROTO_GSTBL_ENTRY   PROTO_GSTBL_ENTRIES[ 3 ]   = {
  { 
    str_id: "BITNA_CRT", 
    src: &BITNA._crt, 
    sz: 1, 
    BAR_PROTO_GSTBL_READ_ONLY, 
    get: nullptr, 
    set: nullptr 
  },
  { 
    str_id: "GRAN_ACC_RANGE", 
    src: &GRAN._acc_range, 
    sz: 1, 
    BAR_PROTO_GSTBL_READ_WRITE, 
    get: nullptr, 
    set: [] ( void* src, [[maybe_unused]]int16_t sz ) -> const char* { return GRAN.set_acc_range( ( MPU9250_accRange )*( uint8_t* )src ) ? nullptr : "GRAN_ACC_INVALID_RANGE"; } 
  },
  { 
    str_id: "GRAN_GYR_RANGE", 
    src: &GRAN._gyr_range, 
    sz: 1, 
    BAR_PROTO_GSTBL_READ_WRITE, 
    get: nullptr, 
    set: [] ( void* src, [[maybe_unused]]int16_t sz ) -> const char* { return GRAN.set_gyr_range( ( MPU9250_gyroRange )*( uint8_t* )src ) ? nullptr : "GRAN_GYR_INVALID_RANGE"; } 
  }
};
struct _PROTO : bar_cache_t< 128 > {
  int init( void ) {
    stream.bind( BAR_PROTO_GSTBL{ 
      entries: PROTO_GSTBL_ENTRIES, 
      size: sizeof( PROTO_GSTBL_ENTRIES ) / sizeof( BAR_PROTO_GSTBL_ENTRY ) 
    } );

    stream.bind( BAR_PROTO_SRWRAP{
      send: [] BAR_PROTO_STREAM_SEND_LAMBDA {
        return COM.blue_itr_send( src, sz );
      },
      recv: [] BAR_PROTO_STREAM_RECV_LAMBDA {
        return COM.blue_itr_recv( src, sz );
      } 
    } );

    stream.bind( [] ( void ) -> int32_t { return PARAMS._bar_proto_seq; } );

    return 0;
  }

  int _out_cache_write( void ) {
    return COM.blue_itr_send( (uint8_t*)_buffer, sizeof( *head ) + head->_dw2.sz );
  }

  BAR_PROTO_STREAM< 256 >   stream;

  int loop( void ) {
    if( COM.blue.available() ) {
      BAR_PROTO_STREAM_RESOLVE_RECV_INFO info;
      if( int ret = stream.resolve_recv( &info ); ret != 0 ) {
        SERIAL_LOG( LOG_ERROR ) << "Protocol fault " << BAR_PROTO_STREAM_ERR_STR[ info.err ] << ".\n";
        return ret;
      }

      if( info.nak_reason != nullptr ) {
        SERIAL_LOG() << "Responded with NAK on seuqence ( " << info.recv_head._dw1.seq << " ), operation ( " << ( int )info.recv_head._dw0.op << " ). Reason: " << info.nak_reason << ".\n";
        return 0;
      }

      switch( info.recv_head._dw0.op ) {
        case BAR_PROTO_OP_PING: {
          SERIAL_LOG() << "Ping'd back on sequence ( " << info.recv_head._dw1.seq << " ).\n";
        break; }
      }
    }

    return 0;
  }

} PROTO;


void _dead( void ) {
  while( 1 ) BITNA.blink( LED::RED, false, 1, 1000, 1000 );
}


int do_tests( void ) {
  SERIAL_LOG( LOG_INFO ) << "Beginning tests.\n";

  const auto _get_test = [] () -> int8_t {
    DYNAMIC.scan();
    return ( DYNAMIC.giselle.dwn << 3 ) | ( DYNAMIC.karina.dwn << 2 ) | ( DYNAMIC.ningning.dwn << 1 ) | DYNAMIC.winter.dwn;
  };

  const auto _begin = [] ( const char* test_name ) -> void {
    SERIAL_LOG( LOG_INFO ) << "Acknowledged test [ " << test_name << " ]... ";
    BITNA.blink( LED::TRQ, true, 10, 50, 50 );
  };
  const auto _end = [] () -> void {
    BITNA.blink( LED::TRQ, true, 10, 50, 50 );
    SERIAL_LOG << "ok.\n";
  };

  int test_count = 0;

l_test_begin: {
    BITNA.blink( LED::TRQ, false, 1, 100, 500 );
    int8_t test = _get_test();

    switch( test ) {
      case 0b0000: goto l_test_begin;
      case 0b1001: goto l_test_end;

      case 0b0100: {
        _begin( "STATUS-LED" ); BITNA.test_rgb( 2000 ); _end();
      break; }

      default: goto l_test_begin;
    }

    ++test_count;
    goto l_test_begin;
}
l_test_end:
  SERIAL_LOG( LOG_INFO ) << "Tests ended. Completed ( " << test_count << " ) tests.\n";
  return test_count;
}


#define SETUP_CRITICAL_ASSERT( c ) if( !c ) { SERIAL_LOG( LOG_CRITICAL ) << "CRITICAL ASSERT ( " #c " ) FAILED. ENTERING DEAD STATE.\n"; _dead(); }
void setup( void ) {
  Serial.begin( 115200 );

  SETUP_CRITICAL_ASSERT( GPIO.init() == 0 );
  
  BITNA.blink( LED::RED, true, 10, 50, 50 );

  SETUP_CRITICAL_ASSERT( COM.init() == 0 );

  SETUP_CRITICAL_ASSERT( DYNAMIC.init() == 0 );

  PARAMS.reset();
  PROTO.init();

  SERIAL_LOG( LOG_OK ) << "Setup complete.\n";
  BITNA.blink( LED::GRN, true, 10, 50, 50 );

  SERIAL_LOG( LOG_INFO ) << "Scanning for tests request.\n";
  DYNAMIC.scan();
  if( DYNAMIC.giselle.dwn ) {
    do_tests();
  } else {
    SERIAL_LOG( LOG_INFO ) << "No tests request.\n";
  }

  SERIAL_LOG( LOG_OK ) << "Ready to work.\n";

  BITNA( LED::BLU );
}

void loop( void ) {
  if( COM.blue.connected() ) {
    if( PARAMS._conn_rst ) {
      PARAMS._conn_rst = false;
      SERIAL_LOG( LOG_INFO ) << "Connected to device.\n";
      BITNA.blink( LED::BLU, true, 10, 50, 50 );
    }

    PROTO.loop();
    
    DYNAMIC.scan();
    DYNAMIC.blue_tx();

    delay( PARAMS.main_loop_delay );

  } else {
    if( !PARAMS._conn_rst ) {
      SERIAL_LOG( LOG_INFO ) << "Disconnected from device.\n";
      PARAMS.reset();
      BITNA.blink( LED::BLU, true, 10, 50, 50 );
    }
    BITNA.blink( LED::BLU, 1, true, 500, 500 );
  }
}