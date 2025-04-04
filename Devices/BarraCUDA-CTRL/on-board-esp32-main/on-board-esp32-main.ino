#include "../barracuda-ctrl.hpp"

#include <MPU6050_WE.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include "../../IXN/ino_common_utils.hpp"
#define WJP_ARCHITECTURE_LITTLE
#include "../../IXN/ino_wjp_on_bths_driver.hpp"
using namespace ixN::Ino;


typedef   int8_t   GPIO_pin_t;
typedef   int8_t   LED_RGB;


static struct _PARAMS {
  void reset() {
    _printf( LogLevel_Info, "Resetting parameters... " );
    *this = _PARAMS{};
    _printf( "ok.\n" );
  }

  int       dynamic_burst_delay     = 50;

} PARAMS;


static struct _GPIO { 
  const int   ADC_nmax = 4095;
  const float ADC_fmax = 4095.0f;

  struct { const GPIO_pin_t sw, x, y; } rachel{ sw: 27, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
  /*       JOYSTICKS                    |lower left                     |upper right */

  const GPIO_pin_t giselle = 5, karina = 18, ningning = 19, winter = 23;
  /*       SWS     |blue        |red         |yellow        |green */

  const GPIO_pin_t naksu = 15;
  /*               |light sensor */

  const GPIO_pin_t kazuha = 4;
  /*               |potentiometer */

  struct { const GPIO_pin_t r, g, b; } bitna{ r: 13, g: 12, b: 14 };

  int init() {
    _printf( LogLevel_Info, "Configuring pin modes... " );

    pinMode( rachel.sw, INPUT_PULLUP ); pinMode( rachel.x, INPUT ); pinMode( rachel.y, INPUT );
    pinMode( samantha.sw, INPUT_PULLUP ); pinMode( samantha.x, INPUT ); pinMode( samantha.y, INPUT );

    pinMode( giselle,  INPUT_PULLUP );
    pinMode( karina,   INPUT_PULLUP );
    pinMode( ningning, INPUT_PULLUP );
    pinMode( winter,   INPUT_PULLUP );

    pinMode( naksu, INPUT );
    pinMode( kazuha, INPUT );

    pinMode( bitna.r, OUTPUT ); pinMode( bitna.g, OUTPUT ); pinMode( bitna.b, OUTPUT );

    _printf( "ok.\n" );

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


struct _GRAN : public MPU6050_WE {
  int init( void ) {
    _printf( LogLevel_Info, "MPU6050 init... " );
    if( !this->MPU6050_WE::init() ) {
      _printf( "fault.\n" );
      return -1;
    }
    _printf( "ok.\n" );

    _printf( LogLevel_Info, "MPU6050 calibrate... " );
    this->MPU6050_WE::setAccRange( _acc_range );
    this->MPU6050_WE::setGyrRange( _gyr_range );
    _printf( "ok.\n" );

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
  enum _IDX : LED_RGB { BLK = 0b000, RED = 0b100, YLW = 0b110, GRN = 0b010, TRQ = 0b011, BLU = 0b001, PRP = 0b101, WHT = 0b111 };

  LED_RGB _crt;

  int set( LED_RGB rgb ) {
    digitalWrite( GPIO.bitna.r, ( rgb >> 2 ) & 1 );
    digitalWrite( GPIO.bitna.g, ( rgb >> 1 ) & 1 );
    digitalWrite( GPIO.bitna.b, ( rgb ) & 1 );
    _crt = rgb;
    return 0;
  }

  int operator () ( LED_RGB rgb ) { return this->set( rgb ); }

  int blink( LED_RGB on_rgb, LED_RGB off_rgb, LED_RGB leave_rgb, int N, int ms_on, int ms_off ) {
    for( int n = 1; n <= N; ++n ) {
      this->set( on_rgb ); delay( ms_on ); this->set( off_rgb ); delay( ms_off );
    }
    this->set( leave_rgb );
    return 0;
  }

  void test_rgb( int ms ) {
    LED_RGB prev = _crt;

    for( LED_RGB rgb = 0; rgb < 7; ++rgb ) {
      this->set( rgb ); delay( ms );
    }

    this->set( prev );
  }

} BITNA;


struct _DYNAMIC : WJP_HEAD, barcud_ctrl::dynamic_t {
  struct {
    struct { float x, y; } rachel, samantha;
  } _idle_reads;

  int init( void ) {
    _printf( LogLevel_Info, "Proto head init... " );
    WJP_HEAD::_dw0.op  = WJPOp_IBurst;
    WJP_HEAD::_dw1.seq = 0;
    WJP_HEAD::_dw3.sz  = sizeof( barcud_ctrl::dynamic_t ); 
    _printf( "ok.\n" );

    _printf( LogLevel_Info, "Joysticks calibrate... " );
    _idle_reads.rachel.x = analog_read_mean< float >( GPIO.rachel.x, 10, 1 ); _idle_reads.rachel.y = analog_read_mean< float >( GPIO.rachel.y, 10, 1 );
    _idle_reads.samantha.x = analog_read_mean< float >( GPIO.samantha.x, 10, 1 ); _idle_reads.samantha.y = analog_read_mean< float >( GPIO.samantha.y, 10, 1 );
    _printf( "ok.\n" );

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
    

    const auto _resolve_switch = [] ( barcud_ctrl::switch_t& sw, GPIO_pin_t pin ) -> void {
      int is_dwn = !digitalRead( pin );

      switch( ( sw.dwn << 1 ) | is_dwn ) {
        /* case 0b00: [[fallthrough]]; */
        /* case 0b11: sw.rls = 0; sw.prs = 0; break; */
        case 0b01: /* sw.rls = 0; */ sw.prs = 1; break;
        case 0b10: sw.rls = 1; /* sw.prs = 0; */ break;
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

    naksu.lvl = 1.0 - sqrt( analogRead( GPIO.naksu ) / GPIO.ADC_fmax );
    kazuha.lvl = analogRead( GPIO.kazuha ) / GPIO.ADC_fmax;
  }

  void reset_sw_prs_rls() {
    rachel.sw.rls = 0; rachel.sw.prs = 0;
    samantha.sw.rls = 0; samantha.sw.prs = 0;
    giselle.rls = 0; giselle.prs = 0;
    karina.rls = 0; karina.prs = 0;
    ningning.rls = 0; ningning.prs = 0;
    winter.rls = 0; winter.prs = 0;
  }

  int blue_tx( void );

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


static WJP_QGSTBL_ENTRY QGSTBL[ 3 ] = {
  { 
    str_id: "BITNA_CRT", 
    sz: 1, 
    WJP_QGSTBL_READ_ONLY, 
    qset_func: nullptr,
    qget_func: nullptr, 
    src: &BITNA._crt
  },
  { 
    str_id: "GRAN_ACC_RANGE", 
    sz: 1, 
    WJP_QGSTBL_READ_WRITE, 
    qset_func: [] WJP_QSET_LAMBDA { return GRAN.set_acc_range( ( MPU9250_accRange )*( uint8_t* )args.addr ) ? nullptr : "GRAN_ACC_INVALID_RANGE"; },
    qget_func: nullptr,
    src: &GRAN._acc_range
  },
  { 
    str_id: "GRAN_GYR_RANGE", 
    sz: 1, 
    WJP_QGSTBL_READ_WRITE, 
    qset_func: [] WJP_QSET_LAMBDA { return GRAN.set_gyr_range( ( MPU9250_gyroRange )*( uint8_t* )args.addr ) ? nullptr : "GRAN_GYR_INVALID_RANGE"; },
    qget_func: nullptr,
    src: &GRAN._gyr_range
  }
};
struct {
  WJP_on_BluetoothSerial   wjblu;

  int init() {
    wjblu.bind_qgstbl( WJP_QGSTBL{ 
      entries: QGSTBL, 
      count: sizeof( QGSTBL ) / sizeof( WJP_QGSTBL_ENTRY ) 
    } );
    wjblu.begin( barcud_ctrl::DEVICE_NAME, 0 );

    _printf( LogLevel_Info, "I^2C wire begin... " );
    Wire.begin();
    _printf( "ok.\n" );
    
    return 0;
  }

  int loop( void ) {
    if( !wjblu.available() ) return 0;

    WJP_RESOLVE_RECV_INFO info;
    if( int ret = wjblu.resolve_recv( &info ); ret <= 0 ) {
      _printf( LogLevel_Error, "Protocol breach: %s.\n ", WJP_err_strs[ info.err ] );
      return ret;
    }

    if( info.nakr != nullptr ) {
      _printf( LogLevel_Info, "Responded with NAK on seq (%d), op (%d). Reson: %s.\n ", ( int )info.recv_head._dw1.seq, ( int )info.recv_head._dw0.op, info.nakr );
      return 0;
    }

    switch( info.recv_head._dw0.op ) {
      case WJPOp_Ping: {
        _printf( LogLevel_Info, "Ping'd back on sequence (%d).\n", ( int )info.recv_head._dw1.seq );
      break; }
    }

    return 0;
  }

} COM;


void _dead( void ) {
  COM.wjblu.end();
  Wire.end();
  while( 1 ) BITNA.blink( LED::RED, LED::BLK, LED::BLK, 1, 1000, 1000 );
}


int do_tests( void ) {
  _printf( LogLevel_Info, "Beginning tests.\n" );

  const auto _get_test = [] () -> int8_t {
    DYNAMIC.scan();
    return ( DYNAMIC.giselle.dwn << 3 ) | ( DYNAMIC.karina.dwn << 2 ) | ( DYNAMIC.ningning.dwn << 1 ) | DYNAMIC.winter.dwn;
  };

  const auto _begin = [] ( const char* test_name ) -> void {
    _printf( LogLevel_Info, "Acknowledged test [ %s ]...", test_name );
    BITNA.blink( LED::TRQ, LED::BLK, LED::BLK, 10, 50, 50 );
  };
  const auto _end = [] () -> void {
    BITNA.blink( LED::TRQ, LED::BLK, LED::BLK, 10, 50, 50 );
    _printf( " ok.\n" );
  };

  int test_count = 0;

l_test_begin: {
    BITNA.blink( LED::TRQ, LED::BLK, LED::TRQ, 1, 100, 500 );
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
  _printf( LogLevel_Info, "Tests ended. Completed (%d) tests.\n", test_count ) ;
  return test_count;
}


#define SETUP_CRITICAL_ASSERT( c ) if( !c ) { _printf( LogLevel_Critical, "CRITICAL ASSERT ( " #c " ) FAILED. ENTERING DEAD STATE.\n" ); _dead(); }
void setup( void ) {
  Serial.begin( 115200 );

  SETUP_CRITICAL_ASSERT( GPIO.init() == 0 );
  
  BITNA.blink( LED::RED, LED::BLK, LED::BLK, 10, 50, 50 );

  SETUP_CRITICAL_ASSERT( COM.init() == 0 );

  SETUP_CRITICAL_ASSERT( DYNAMIC.init() == 0 );

  PARAMS.reset();

  _printf( LogLevel_Ok, "Setup complete.\n" );
  BITNA.blink( LED::GRN, LED::BLK, LED::BLK, 10, 50, 50 );

  _printf( LogLevel_Info, "Scanning for tests request.\n" );
  DYNAMIC.scan();
  if( DYNAMIC.giselle.dwn ) {
    do_tests();
  } else {
    _printf( LogLevel_Info, "No tests request.\n" );
  }

  _printf( LogLevel_Ok, "Ready for link...\n" );

  BITNA( LED::BLU );
}


struct _LOOP_STATE {
  bool   _conn_rst   = true;
} LOOP_STATE;

/* 1: LOOP_STATE._conn_rst | 0: COM.wjblu.connected() */
std::function< int( void ) >   loop_procs[]   = {
  /* 0b00 */ [] () -> bool {
    _printf( LogLevel_Info, "Disconnected from device.\n" );
    PARAMS.reset();
    LOOP_STATE._conn_rst = true;
    _printf( LogLevel_Info, "Ready to relink...\n" );
    BITNA.blink( LED::BLU, LED::RED, LED::BLU, 10, 50, 50 );
    return true;
  },
  /* 0b01 */ [] () -> bool {
    if( COM.loop() != 0 ) {
      BITNA.blink( LED::RED, LED::BLK, LED::RED, 9, 100, 500 );
      COM.wjblu.disconnect();
      return true;
    }
    
    DYNAMIC.scan();
    DYNAMIC.blue_tx();

    return true;
  },
  /* 0b10 */ [] () -> bool {
    BITNA.blink( LED::BLU, LED::BLK, LED::BLU, 1, 100, 500 );
    return true;
  },
  /* 0b11 */ [] () -> bool {
    LOOP_STATE._conn_rst = false;
    _printf( LogLevel_Info, "Connected to device.\n" );
    BITNA.blink( LED::BLU, LED::BLK, LED::BLU, 10, 50, 50 );
    return true;
  }
};

void loop( void ) {
  loop_procs[ ( LOOP_STATE._conn_rst << 1 ) | COM.wjblu.connected() ]();
}


int _DYNAMIC::blue_tx( void ) {
  static int last_ms = millis();
  static int current_ms;

  current_ms = millis();
  if( current_ms - last_ms < PARAMS.dynamic_burst_delay ) return 0;

  last_ms = current_ms;
  int ret = COM.wjblu.trust_burst( this, sizeof( WJP_HEAD ) + WJP_HEAD::_dw3.sz, 0 );

  this->reset_sw_prs_rls();
  return ret;
}
