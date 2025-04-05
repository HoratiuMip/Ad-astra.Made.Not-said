#include "../barracuda-ctrl.hpp"
#include "logo.hpp"

#include <Wire.h>
#include <MPU6050_WE.h>

#define SSD1306_NO_SPLASH
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../../IXN/common_utils.hpp"
#define WJP_ARCHITECTURE_LITTLE
#include "../../IXN/Driver/wjp_on_bths.hpp"
using namespace ixN::uC;

#include <atomic>



typedef   int8_t   GPIO_pin_t;

enum Mode_ : int {
    Mode_Game = 0,
    Mode_Ctrl = 1
};



static struct _CONFIG {
    std::atomic_int   mode      = { Mode_Game };

    TwoWire*          I2C_bus   = &Wire;

} CONFIG;

static struct _CONFIG_CTRL_MODE {
    void reset() {
        _printf( "Control mode configuration reset... " );
        *this = _CONFIG_CTRL_MODE{};
        _printf( "ok.\n" );
    }

    int   dynamic_period   = 10;

} CONFIG_CTRL_MODE;



struct GPIO { 
    inline static const int     ADC_nmax   = 4095;
    inline static const float   ADC_fmax   = 4095.0f;

    inline static struct { const GPIO_pin_t sw, x, y; } rachel{ sw: 27, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
    /*                                                  |lower left                     |upper right */

    inline static const GPIO_pin_t giselle = 5, karina = 18, ningning = 23, winter = 19;
    /*                             |blue        |red         |yellow        |green */

    inline static const GPIO_pin_t naksu = 15;
    /*                             |light sensor */

    inline static const GPIO_pin_t kazuha = 4;
    /*                             |potentiometer */

    inline static struct { const GPIO_pin_t r, g, b; } bitna{ r: 13, g: 12, b: 14 };
    /*                                                 |led */

    /**
     * @brief Configures the pins' modes.
     * @returns `0` on success, negative otherwise.
     * @attention Should always return `0`.
     */
    static int init( void ) {
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

    inline static int d_r( GPIO_pin_t pin ) { return digitalRead( pin ); }
    inline static void d_w( GPIO_pin_t pin, int level ) { digitalWrite( pin, level ); }
    inline static int a_r( GPIO_pin_t pin ) { return analogRead( pin ); }
    inline static void a_w( GPIO_pin_t pin ); 

    /**
     * @brief Performs N analog reads on pin.
     * @returns The mean of the `N` reads, as `_T`, in the range [ `0,  2^{ADC resolution}-1` ].
     * @param[ in ] pin: The pin number.
     * @param[ in ] N: The number of reads to perform.
     */
    template< typename _T > _T static a_rm( GPIO_pin_t pin, int N ) {
        int acc = 0;
        for( int n = 1; n <= N; ++n ) acc += a_r( pin );
        return ( _T )acc / N;
    }

};



#define Led_BLK 0b000
#define Led_RED 0b100
#define Led_YLW 0b110
#define Led_GRN 0b010
#define Led_TRQ 0b011
#define Led_BLU 0b001
#define Led_PRP 0b101
#define Led_WHT 0b111
typedef   int8_t   Led_;
static struct _BITNA {
    Led_   _crt   = Led_BLK;
    
    /**
     * @brief Sets the accelerometer's range.
     * @param[ in ] rgb: The RGB value to write.
     */
    _BITNA& set( Led_ rgb ) {
        GPIO::d_w( GPIO::bitna.r, ( rgb >> 2 ) & 1 );
        GPIO::d_w( GPIO::bitna.g, ( rgb >> 1 ) & 1 );
        GPIO::d_w( GPIO::bitna.b, ( rgb ) & 1 );
        _crt = rgb;
        return *this;
    }
    
    /**
     * @brief Calls `this->set( rgb )`.
     */
    inline _BITNA& operator () ( Led_ rgb ) { return this->set( rgb ); }

    /**
     * @brief OR with the current color and write it.
     */
    inline _BITNA& operator |= ( Led_ rgb ) { return this->set( _crt | rgb ); }
    /**
     * @brief AND with the current color and write it.
     */
    inline _BITNA& operator &= ( Led_ rgb ) { return this->set( _crt & rgb ); }
    /**
     * @brief XOR with the current color and write it.
     */
    inline _BITNA& operator ^= ( Led_ rgb ) { return this->set( _crt ^ rgb ); }

    /**
     * @brief Blink the LED.
     * @attention The LED color is left as `off_rgb` after this function returns.
     * @param[ in ] N: The number of cycles.
     * @param[ in ] on/off_rgb: The RGB during the `ON/OFF` period. 
     * @param[ in ] ms_on/off: The `ON/OFF` period duration, in `ms`.
     */
    _BITNA& blink( int N, Led_ on_rgb, Led_ off_rgb, int ms_on, int ms_off ) {
        for( int n = 1; n <= N; ++n ) {
            this->set( on_rgb ); vTaskDelay( ms_on ); this->set( off_rgb ); vTaskDelay( ms_off );
        }
        return *this;
    }

    /**
     * @brief Cycles through the 8 possible colors.
     * @attention This function preserves the color of the LED before the call.
     * @param[ in ] ms: The duration of each color, in `ms`.
     */
    void test_rgb( int ms ) {
        Led_ prev = _crt;

        for( int8_t rgb = 0; rgb <= 7; ++rgb ) {
            this->set( ( Led_ )rgb ); vTaskDelay( ms );
        }

        this->set( prev );
    }

} BITNA;

static struct _YUNA : public Adafruit_SSD1306 {
    using Adafruit_SSD1306::Adafruit_SSD1306;
    using Adafruit_SSD1306::WIDTH;
    using Adafruit_SSD1306::HEIGHT;
    
    /**
     * @brief Configures the display.
     * @returns `0` on success, negative otherwise.
     */
    int init( void ) {
        if( !this->begin( SSD1306_SWITCHCAPVCC, 0x3C ) ) return -1;
        this->setTextColor( SSD1306_WHITE );
        this->setTextWrap( false );
        this->clearDisplay();
        this->drawBitmap(0, 0, BARRACUDA_CTRL_LOGO, 128, 64, 1);
        this->display(); this->invertDisplay(1);
        
        return 0;
    }

    static void splash( void* flag );

} YUNA{ 128, 64, CONFIG.I2C_bus, -1 };

static struct _GRAN : public MPU6050_WE {
    using MPU6050_WE::MPU6050_WE;

    /**
     * @brief Configures the gyroscope and accelerometer ( MPU-6050 chip ).
     * @returns 0 on success, negative otherwise.
     */
    int init( void ) {
        _printf( LogLevel_Info, "MPU6050 init... " );
        if( !this->MPU6050_WE::init() ) {
        _printf( "fault.\n" );
        return -1;
        }
        _printf( "ok.\n" );

        _printf( LogLevel_Info, "MPU6050 calibrate... " );
        this->MPU6050_WE::setAccRange( MPU6050_ACC_RANGE_2G );
        this->MPU6050_WE::setGyrRange( MPU6050_GYRO_RANGE_250 );
        _printf( "ok.\n" );

        return 0;
    }

    /**
     * @brief Sets the accelerometer's range.
     * @returns 0 on success, negative otherwise.
     */
    bool set_acc_range( MPU9250_accRange range ) {
        if( range < MPU6050_ACC_RANGE_2G || range > MPU6050_ACC_RANGE_16G ) return false;
        this->MPU6050_WE::setAccRange( range );
        return true;
    }

    /**
     * @brief Sets the gyroscope's range.
     * @returns 0 on success, negative otherwise.
     */
    bool set_gyr_range( MPU9250_gyroRange range ) {
        if( range < MPU6050_GYRO_RANGE_250 || range > MPU6050_GYRO_RANGE_2000 ) return false;
        this->MPU6050_WE::setGyrRange( range );
        return true;
    }

} GRAN{ CONFIG.I2C_bus, MPU6050_WE::WHO_AM_I_CODE };

#define _DYNAM_SCAN_IF( bit, jmp_lbl ) if( ( flags & ( 1 << bit ) ) == 0 ) goto jmp_lbl;
enum Scan_ : int {
    Scan_Joysticks   = ( 1 << 0 ),
    Scan_Switches    = ( 1 << 1 ),
    Scan_Accel       = ( 1 << 2 ),
    Scan_Gyro        = ( 1 << 3 ),
    Scan_Light       = ( 1 << 4 ),
    Scan_Potentio    = ( 1 << 5 ),

    Scan_All         = ~0
};
static struct _DYNAM : WJP_HEAD, barcud_ctrl::dynamic_t {
    struct {
        struct { float x, y; } rachel, samantha;
    } _idle_reads;

    /**
     * @brief Configures parameters for proper dynam flow.
     * @returns `0` on success, negative otherwise.
     */
    int init( void ) {
        int status = 0;

        _printf( LogLevel_Info, "Proto head init... " );
        WJP_HEAD::_dw0.op  = WJPOp_IBurst;
        WJP_HEAD::_dw1.seq = 0;
        WJP_HEAD::_dw3.sz  = sizeof( barcud_ctrl::dynamic_t ); 
        _printf( "ok.\n" );

        _printf( LogLevel_Info, "Joysticks calibrate... " );
        _idle_reads.rachel.x = GPIO::a_rm< float >( GPIO::rachel.x, 10 ); _idle_reads.rachel.y = GPIO::a_rm< float >( GPIO::rachel.y, 10 );
        _idle_reads.samantha.x = GPIO::a_rm< float >( GPIO::samantha.x, 10 ); _idle_reads.samantha.y = GPIO::a_rm< float >( GPIO::samantha.y, 10 );
        _printf( "ok.\n" );

        status = GRAN.init(); if( status != 0 ) return status;
        //status = YUNA.init(); if( status != 0 ) return status;

        return status;
    }

    /**
     * @brief Updates the dynam values.
     * @param[ in ] flags: Flags to select function behaviour. Default, updates everything.
     */
    void scan( int flags = Scan_All ) {
    /* Joysticks */ l_joysticks: _DYNAM_SCAN_IF( 0, l_switches ); {
        const auto _resolve_joystick_axis = [] ( float& read, float idle_read ) static -> void {
            if( ( read -= idle_read ) < 0.0f ) 
                read /= idle_read;
            else
                read /= GPIO::ADC_fmax - idle_read;
        };

        rachel.x = GPIO::a_r( GPIO::rachel.x ); rachel.y = GPIO::a_r( GPIO::rachel.y );
        samantha.x = GPIO::a_r( GPIO::samantha.x ); samantha.y = GPIO::a_r( GPIO::samantha.y );

        _resolve_joystick_axis( rachel.x, _idle_reads.rachel.x ); _resolve_joystick_axis( rachel.y, _idle_reads.rachel.y );
        _resolve_joystick_axis( samantha.x, _idle_reads.samantha.x ); _resolve_joystick_axis( samantha.y, _idle_reads.samantha.y );

        samantha.y *= -1.0f;
    }
    /* Switches */ l_switches: _DYNAM_SCAN_IF( 1, l_accel ); {
        const auto _resolve_switch = [] ( barcud_ctrl::switch_t& sw, GPIO_pin_t pin ) static -> void {
        int is_dwn = !digitalRead( pin );

        switch( ( sw.dwn << 1 ) | is_dwn ) {
            /* case 0b00: [[fallthrough]]; */
            /* case 0b11: sw.rls = 0; sw.prs = 0; break; */
            case 0b01: /* sw.rls = 0; */ sw.prs = 1; break;
            case 0b10: sw.rls = 1; /* sw.prs = 0; */ break;
        }

        sw.dwn = is_dwn;
        };

        _resolve_switch( rachel.sw,   GPIO::rachel.sw );
        _resolve_switch( samantha.sw, GPIO::samantha.sw );
        _resolve_switch( giselle,     GPIO::giselle );
        _resolve_switch( karina,      GPIO::karina );
        _resolve_switch( ningning,    GPIO::ningning );
        _resolve_switch( winter,      GPIO::winter);
    }
    /* Acceleration */ l_accel: _DYNAM_SCAN_IF( 2, l_gyro ); {
        xyzFloat acc_read = GRAN.getGValues();
        gran.acc = { x: acc_read.y, y: -acc_read.x, z: acc_read.z };
    }
    /* Gyroscope */ l_gyro: _DYNAM_SCAN_IF( 3, l_light ); {
        xyzFloat gyr_read = GRAN.getGyrValues();
        gran.gyr = { x: gyr_read.y, y: -gyr_read.x, z: gyr_read.z };
    }
    /* Light */ l_light: _DYNAM_SCAN_IF( 4, l_potentio ); {
        naksu.lvl = 1.0 - sqrt( analogRead( GPIO::naksu ) / GPIO::ADC_fmax );
    }
    /* Potentiometer */ l_potentio: _DYNAM_SCAN_IF( 5, l_end ); {
        kazuha.lvl = analogRead( GPIO::kazuha ) / GPIO::ADC_fmax;
    }
    l_end: return;
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

} DYNAM;



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
    src: nullptr
  },
  { 
    str_id: "GRAN_GYR_RANGE", 
    sz: 1, 
    WJP_QGSTBL_READ_WRITE, 
    qset_func: [] WJP_QSET_LAMBDA { return GRAN.set_gyr_range( ( MPU9250_gyroRange )*( uint8_t* )args.addr ) ? nullptr : "GRAN_GYR_INVALID_RANGE"; },
    qget_func: nullptr,
    src: nullptr
  }
};
struct {
  WJP_on_BluetoothSerial   wjblu;

  int init( void ) {
    _printf( LogLevel_Info, "I^2C wire begin... " );
    Wire.begin();
    _printf( "ok.\n" );

    YUNA.init();
    
    wjblu.bind_qgstbl( WJP_QGSTBL{ 
      entries: QGSTBL, 
      count: sizeof( QGSTBL ) / sizeof( WJP_QGSTBL_ENTRY ) 
    } );
    wjblu.begin( barcud_ctrl::DEVICE_NAME, 0 );

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
  while( 1 ) BITNA.blink( 1, Led_RED, Led_BLK, 1000, 1000 );
}


int do_tests( void ) {
    _printf( LogLevel_Info, "Beginning tests.\n" );

    const auto _get_test = [] () static -> int8_t {
        DYNAM.scan( Scan_Switches );
        return ( DYNAM.giselle.dwn << 3 ) | ( DYNAM.karina.dwn << 2 ) | ( DYNAM.ningning.dwn << 1 ) | DYNAM.winter.dwn;
    };

    const auto _begin = [] ( const char* test_name ) static -> void {
        _printf( LogLevel_Info, "Acknowledged test [ %s ]...", test_name );
        BITNA.blink( 10, Led_TRQ, Led_BLK, 50, 50 );
    };
    const auto _end = [] () static -> void {
        BITNA.blink( 10, Led_TRQ, Led_BLK, 50, 50 );
        _printf( " ok.\n" );
    };

    int test_count = 0;

l_test_begin: {
    BITNA.blink( 1, Led_TRQ, Led_BLK, 100, 500 );
    int8_t test = _get_test();

    switch( test ) {
        case 0b0000: goto l_test_begin;
        case 0b1001: goto l_test_end;

        case 0b0100: {
            _begin( "BITNA - LED color cycle" ); BITNA.test_rgb( 2000 ); _end();
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


#define _SETUP_ASSERT_OR_DEAD( c ) if( !(c) ) { _printf( LogLevel_Critical, "SETUP ASSERT ( " #c " ) FAILED. ENTERING DEAD STATE.\n" ); _dead(); *(int*)0=*(int*)-1=*(int*)1; }
void setup( void ) {
    _SETUP_ASSERT_OR_DEAD( GPIO::init() == 0 );
    BITNA( Led_RED ); vTaskDelay( 300 );

    Serial.begin( 115200 );

    _SETUP_ASSERT_OR_DEAD( YUNA.init() == 0 );
    BITNA |= Led_BLU; vTaskDelay( 300 );

    TaskHandle_t task;
    auto status = xTaskCreatePinnedToCore(
        &_YUNA::splash,
        "YUNA splash",
        4096,
        nullptr,
        0,
        &task,
        0
    );
                    
    _SETUP_ASSERT_OR_DEAD( COM.init() == 0 );

    _SETUP_ASSERT_OR_DEAD( DYNAM.init() == 0 );

    CONFIG_CTRL_MODE.reset();

    _printf( LogLevel_Ok, "Setup complete.\n" );
    BITNA.blink( 10, Led_GRN, Led_BLK, 50, 50 );

    _printf( LogLevel_Info, "Scanning for tests request.\n" );
    DYNAM.scan( Scan_Switches );
    if( DYNAM.giselle.dwn ) {
        do_tests();
    } else {
        _printf( LogLevel_Info, "No tests request.\n" );
    }

    _printf( LogLevel_Ok, "Ready for link...\n" );

    BITNA( Led_BLU );
}


struct _LOOP_STATE {
  bool   _conn_rst   = true;
} LOOP_STATE;

/* 1: LOOP_STATE._conn_rst | 0: COM.wjblu.connected() */
std::function< int( void ) >   loop_procs[]   = {
  /* 0b00 */ [] () -> bool {
    _printf( LogLevel_Info, "Disconnected from device.\n" );
    CONFIG_CTRL_MODE.reset();
    LOOP_STATE._conn_rst = true;
    _printf( LogLevel_Info, "Ready to relink...\n" );
    BITNA.blink( 10, Led_BLU, Led_RED, 50, 50 );
    return true;
  },
  /* 0b01 */ [] () -> bool {
    if( COM.loop() != 0 ) {
      BITNA.blink( 10, Led_RED, Led_BLK, 100, 500 );
      COM.wjblu.disconnect();
      return true;
    }
    
    DYNAM.scan();
    DYNAM.blue_tx();

    return true;
  },
  /* 0b10 */ [] () -> bool {
    BITNA.blink( 1, Led_BLU, Led_BLK, 100, 500 );
    return true;
  },
  /* 0b11 */ [] () -> bool {
    LOOP_STATE._conn_rst = false;
    _printf( LogLevel_Info, "Connected to device.\n" );
    BITNA.blink( 10, Led_BLU, Led_BLK, 50, 50 );
    return true;
  }
};

void loop( void ) {
  loop_procs[ ( LOOP_STATE._conn_rst << 1 ) | COM.wjblu.connected() ]();
}


int _DYNAM::blue_tx( void ) {
  static int last_ms = millis();
  static int current_ms;

  current_ms = millis();
  if( current_ms - last_ms < CONFIG_CTRL_MODE.dynamic_period ) return 0;

  last_ms = current_ms;
  int ret = COM.wjblu.trust_burst( this, sizeof( WJP_HEAD ) + WJP_HEAD::_dw3.sz, 0 );

  this->reset_sw_prs_rls();
  return ret;
}


void _YUNA::splash( void* flag ) { bool inv = false;
    while( true ) { BITNA ^= Led_GRN; vTaskDelay( 500 ); YUNA.invertDisplay( inv ^= true ); }
}
