/*====== BarraCUDA - Main - Vatca "Mipsan" Tudor-Horatiu 
|
|=== DESCRIPTION
> Ask Daniel. ( massive red flag )
|
======*/
#include "../barracuda.hpp"

#include <Wire.h>
#include <MPU6050_WE.h>

#define SSD1306_NO_SPLASH
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>

#include "../../IXN/common_utils.hpp"
#define WJP_ARCHITECTURE_LITTLE
#include "../../IXN/Driver/wjp_on_bths.hpp"
using namespace ixN::uC;

#include "graphics.hpp"

#include <atomic>


#define uC_CORE_0 0
#define uC_CORE_1 1

typedef   int8_t   GPIO_pin_t;

enum Mode_ : int {
    Mode_Brdg = 0,
    Mode_Game = 1,
    Mode_Ctrl = 2,

    _Mode_Count
};

enum Priority_ : int {
    Priority_Aesth = 0,
    Priority_Urgent = 5
};



#define MAIN_LOOP_ON( _M ) for(; CONFIG.mode == _M ;)
static struct _CONFIG {
	int init( void );

    void ( *mains[ _Mode_Count ] )( void* );

    std::atomic_int     mode         = { Mode_Brdg };

    TwoWire*            I2C_bus      = &Wire;

    SemaphoreHandle_t   init_sem     = { xSemaphoreCreateBinary() };

    int                 dyn_scan_T   = 10;

} CONFIG;

static struct _CONFIG_BRDG_MODE {
    int init( void );

} CONFIG_BRDG_MODE;

static struct _CONFIG_GAME_MODE {
	int init( void );

} CONFIG_GAME_MODE;

static struct _CONFIG_CTRL_MODE {
	int init( void );

	WJP_on_BluetoothSerial   wjpblu           = {};
    int                      dynamic_period   = 10;

} CONFIG_CTRL_MODE;



void main_brdg( void* );
void main_game( void* );
void main_ctrl( void* );



struct GPIO { 
    inline static const int     ADC_nmax   = 4095;
    inline static const float   ADC_fmax   = 4095.0f;

    inline static struct { const GPIO_pin_t sw, x, y; } rachel{ sw: 27, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
    /*                                                  |lower left                     |upper right */

    inline static const GPIO_pin_t giselle = 5, karina = 18, ningning = 23, winter = 19;
    /*                             |blue        |red         |yellow        |green */

    inline static const GPIO_pin_t naksu = 15;
    /*                             |light */

    inline static const GPIO_pin_t kazuha = 4;
    /*                             |potentiometer */

    inline static const GPIO_pin_t xabara = 17;
    /*                             |bridge */

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

        pinMode( xabara, INPUT_PULLUP );
        attachInterrupt( digitalPinToInterrupt( xabara ), [] ( void ) static -> void { 
            CONFIG.mode.store( Mode_Brdg );
        },
            RISING 
        );

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
    _BITNA& itr_rgb( int ms ) {
        Led_ prev = _crt;

        for( Led_ rgb = 0; rgb <= 7; ++rgb ) {
            this->set( rgb ); vTaskDelay( ms );
        }

        return this->set( prev );
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
        this->setFont( &BARRACUDA_GFX_FONT );
        this->setTextColor( SSD1306_WHITE );
        this->setCursor( 0, 0 );
        this->setTextWrap( false );
		this->setTextSize( 1 );
        this->clearDisplay();
        
        return 0;
    }

	inline _YUNA& clear( void ) { this->clearDisplay(); return *this; }

	template< typename _T > inline _YUNA& print_w( _T&& arg ) { this->print( std::forward< _T >( arg ) ); this->display(); return *this; }

    template< typename ..._Args >
    inline _YUNA& printf_w( const char* fmt, _Args&&... args ) {
        static constexpr int BUF_MAX_SIZE = 256;
        char buffer[ BUF_MAX_SIZE ];
        sprintf( buffer, fmt, std::forward< _Args >( args )... );
        return this->print_w( buffer );
    }

	inline void splash_logo() {
		this->drawBitmap( 0, 0, BARRACUDA_LOGO, 128, 64, 1 );
        this->display();
	}

	void _psplash_invert( bool* running ) { bool inv;
		while( 1 ) { BITNA ^= Led_GRN; vTaskDelay( 500 ); this->invertDisplay( inv ^= true ); }
	}

	inline static const int _PSPLASH_COUNT = 1;
	inline static void ( _YUNA::*_PSPLASH_TBL[ _PSPLASH_COUNT ] )( bool* ) = {
		&_YUNA::_psplash_invert
	};

    struct pspl_info_t {
		/* The handle of the invoker task to be notified on splash end. */
		TaskHandle_t                            task      = NULL;
		/* The index of the splash. `-1` for custom splashes. */
		int                                     idx       = -1;
		/* The custom splash function. `NULL` if index is used. Called with this structure. */
		std::function< void( pspl_info_t* ) >   func      = NULL;
		/* The index on which to notify. `-1` for default. */
		int                                     ntf_idx   = 0;
		/* Flag indicating wether the splash shall keep running. */
		bool                                    running   = true;
    };

    static void psplash( void* pspl_info_v );

	inline static int make_psplash( pspl_info_t* pspl_info, int core = 0, int stack_size_w = 4096  ) {
		TaskHandle_t _task;
		if( xTaskCreatePinnedToCore( &_YUNA::psplash, "YUNA::psplash()", stack_size_w, ( void* )pspl_info, 0, &_task, core ) == pdPASS ) return 0;
		return -1;
	}

} YUNA{ 128, 64, CONFIG.I2C_bus, -1 };

void _YUNA::psplash( void* pspl_info_v ) {
	pspl_info_t* pspl_info = ( pspl_info_t* )pspl_info_v;

	struct _NOTIFY_ON_RET {
		~_NOTIFY_ON_RET() { this->proc(); } std::function< void( void ) > proc;
	} _notify_on_ret{ proc: [ &pspl_info ] () -> void { 
		xTaskNotifyGiveIndexed( pspl_info->task, pspl_info->ntf_idx ); 
		vTaskDelete( xTaskGetCurrentTaskHandle() );
	} };

	if( pspl_info->idx >= 0 && pspl_info->idx <= YUNA._PSPLASH_COUNT ) {
		( YUNA.* _PSPLASH_TBL[ pspl_info->idx ] )( &pspl_info->running );
		goto l_end;
	}

l_end:
	return;
}


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


static struct _SUSAN : public Adafruit_BMP280 {
    int init( void ) {
        if( !this->Adafruit_BMP280::begin( 0x76 ) ) return -1;
        return 0;
    }

} SUSAN{ CONFIG.I2C_bus };


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
static struct _DYNAM : WJP_HEAD, barra::dynamic_t {
    struct {
        struct { float x, y; } rachel, samantha;
    } _idle_reads;

    struct snapshot_token_t {
        barra::dynamic_t*   dst    = NULL;
        int                 _seq   = 0;
    };

    int   _seq  = 0;
    struct {
        QueueHandle_t      handle   = NULL;
        StaticQueue_t      header;
        barra::dynamic_t   buffer;
    } _q[ 2 ];

    /**
     * @brief Configures parameters for proper dynam flow.
     * @returns `0` on success, negative otherwise.
     */
    int init( void ) {
        int status = 0;

        _printf( LogLevel_Info, "Proto head init... " );
        WJP_HEAD::_dw0.op  = WJPOp_IBurst;
        WJP_HEAD::_dw1.seq = 0;
        WJP_HEAD::_dw3.sz  = sizeof( barra::dynamic_t ); 
        _printf( "ok.\n" );

        _printf( LogLevel_Info, "Joysticks calibrate... " );
        _idle_reads.rachel.x = GPIO::a_rm< float >( GPIO::rachel.x, 10 ); _idle_reads.rachel.y = GPIO::a_rm< float >( GPIO::rachel.y, 10 );
        _idle_reads.samantha.x = GPIO::a_rm< float >( GPIO::samantha.x, 10 ); _idle_reads.samantha.y = GPIO::a_rm< float >( GPIO::samantha.y, 10 );
        _printf( "ok.\n" );

        _q[ 0 ].handle = xQueueCreateStatic( 1, sizeof( barra::dynamic_t ), ( uint8_t* )&_q[ 0 ].buffer, &_q[ 0 ].header );
        _q[ 1 ].handle = xQueueCreateStatic( 1, sizeof( barra::dynamic_t ), ( uint8_t* )&_q[ 1 ].buffer, &_q[ 1 ].header );
        xQueueOverwrite( _q[ _seq ^ 1 ].handle, &( barra::dynamic_t& )*this );

        return status;
    }

    void _scan( int flags, barra::dynamic_t* tar ) {
        /* Joysticks */ l_joysticks: _DYNAM_SCAN_IF( 0, l_switches ); {
            const auto _resolve_joystick_axis = [] ( float& read, float idle_read ) static -> void {
                if( ( read -= idle_read ) < 0.0f ) 
                    read /= idle_read;
                else
                    read /= GPIO::ADC_fmax - idle_read;
            };

            tar->rachel.x = GPIO::a_r( GPIO::rachel.x ); tar->rachel.y = GPIO::a_r( GPIO::rachel.y );
            tar->samantha.x = GPIO::a_r( GPIO::samantha.x ); tar->samantha.y = GPIO::a_r( GPIO::samantha.y );

            _resolve_joystick_axis( tar->rachel.x, _idle_reads.rachel.x ); _resolve_joystick_axis( tar->rachel.y, _idle_reads.rachel.y );
            _resolve_joystick_axis( tar->samantha.x, _idle_reads.samantha.x ); _resolve_joystick_axis( tar->samantha.y, _idle_reads.samantha.y );

            samantha.y *= -1.0f;
        }
        /* Switches */ l_switches: _DYNAM_SCAN_IF( 1, l_accel ); {
            const auto _resolve_switch = [] ( barra::switch_t& sw, GPIO_pin_t pin ) static -> void {
            int is_dwn = !digitalRead( pin );

            switch( ( sw.dwn << 1 ) | is_dwn ) {
                /* case 0b00: [[fallthrough]]; */
                /* case 0b11: sw.rls = 0; sw.prs = 0; break; */
                case 0b01: /* sw.rls = 0; */ sw.prs = 1; break;
                case 0b10: sw.rls = 1; /* sw.prs = 0; */ break;
            }

            sw.dwn = is_dwn;
            };

            _resolve_switch( tar->rachel.sw,   GPIO::rachel.sw );
            _resolve_switch( tar->samantha.sw, GPIO::samantha.sw );
            _resolve_switch( tar->giselle,     GPIO::giselle );
            _resolve_switch( tar->karina,      GPIO::karina );
            _resolve_switch( tar->ningning,    GPIO::ningning );
            _resolve_switch( tar->winter,      GPIO::winter);
        }
        /* Acceleration */ l_accel: _DYNAM_SCAN_IF( 2, l_gyro ); {
            xyzFloat acc_read = GRAN.getGValues();
            tar->gran.acc = { x: acc_read.y, y: -acc_read.x, z: acc_read.z };
        }
        /* Gyroscope */ l_gyro: _DYNAM_SCAN_IF( 3, l_light ); {
            xyzFloat gyr_read = GRAN.getGyrValues();
            tar->gran.gyr = { x: gyr_read.y, y: -gyr_read.x, z: gyr_read.z };
        }
        /* Light */ l_light: _DYNAM_SCAN_IF( 4, l_potentio ); {
            tar->naksu.lvl = 1.0 - sqrt( analogRead( GPIO::naksu ) / GPIO::ADC_fmax );
        }
        /* Potentiometer */ l_potentio: _DYNAM_SCAN_IF( 5, l_end ); {
            tar->kazuha.lvl = analogRead( GPIO::kazuha ) / GPIO::ADC_fmax;
        }
    l_end:
        return;
    }
    
    /**
     * @brief Updates the dynam values.
     * @param[ in ] flags: Flags to select function behaviour. Default, updates everything.
     */
    inline void scan( int flags ) {
        this->_scan( flags, &( barra::dynamic_t& )*this );
    }

    void q_scan( int flags ) {
        barra::dynamic_t* tar = &_q[ _seq ^ 1 ].buffer;  

        this->_scan( flags, tar );
    
        xQueueReceive( _q[ _seq ^ 1 ].handle, &_q[ _seq ].buffer, portMAX_DELAY );
        xQueueOverwrite( _q[ _seq ].handle, &_q[ _seq ].buffer );

        _seq ^= 1;
    }

    inline int snapshot( snapshot_token_t* tok ) {
        xQueuePeek( _q[ tok->_seq ].handle, tok->dst, portMAX_DELAY );
        tok->_seq ^= 1;
        return 0;
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



struct {

  int loop( void ) {
    if( !CONFIG_CTRL_MODE.wjpblu.available() ) return 0;

    WJP_RESOLVE_RECV_INFO info;
    if( int ret = CONFIG_CTRL_MODE.wjpblu.resolve_recv( &info ); ret <= 0 ) {
      _printf( LogLevel_Error, "Protocol breach: %s.\n ", WJP_err_strs[ info.err ] );
      return ret;
    }

    if( info.nakr != NULL ) {
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
  CONFIG_CTRL_MODE.wjpblu.end();
  Wire.end();
  while( 1 ) BITNA.blink( 1, Led_RED, Led_BLK, 1000, 1000 );
}



int do_tests( void ) {
    _printf( LogLevel_Info, "Beginning tests.\n" );

    bool in_test    = false;
    int  test_count = 0;

    const auto _get_test = [] () static -> int8_t {
        DYNAM.scan( Scan_Switches );
        return ( DYNAM.giselle.dwn << 3 ) | ( DYNAM.karina.dwn << 2 ) | ( DYNAM.ningning.dwn << 1 ) | DYNAM.winter.dwn;
    };

    const auto _begin = [ &in_test ] ( const char* test_name ) -> void {
        _printf( LogLevel_Info, "Acknowledged test [ %s ]...", test_name );
        BITNA.blink( 10, Led_YLW, Led_BLK, 50, 50 );
        in_test = true;
    };
    const auto _end = [ &in_test ] () -> void {
        BITNA.blink( 10, Led_YLW, Led_BLK, 50, 50 );
        _printf( " ok.\n" );
        in_test = false;
    };

    YUNA.print_w( ">/ test" );


    TaskHandle_t htsk_anim; xTaskCreate( [] ( void* arg ) static -> void { 
            bool& in_test  = *( bool* )arg;
            int   at       = 1;

            int x = YUNA.getCursorX(), y = YUNA.getCursorY();

            for(;;) {
                YUNA.print_w( '.' );
                vTaskDelay( 300 );

                if( at++ < 5 ) continue;

                at = 1;
                YUNA.fillRect( x, y, 20, 5, SSD1306_BLACK );
                YUNA.setCursor( x, y );
            }
        }, 
        "test_display_animation", 4096, &in_test, Priority_Aesth, &htsk_anim 
    );

l_test_begin: {
    BITNA.blink( 1, Led_YLW, Led_BLK, 100, 500 );
    int8_t test = _get_test();

    switch( test ) {
        case 0b0000: goto l_test_begin;
        case 0b0110: goto l_test_end;

        case 0b1000: {
            _begin( "BITNA - LED color cycle" ); BITNA.itr_rgb( 2000 ); _end();
        break; }

        default: goto l_test_begin;
    }

    ++test_count;
    goto l_test_begin;
}
l_test_end:
    _printf( LogLevel_Info, "Tests ended. Completed (%d) tests.\n", test_count ) ;

    vTaskDelete( htsk_anim );
    YUNA.printf_w( " >done(%d)", test_count );
    return test_count;
}



#define _SETUP_ASSERT_OR_DEAD( c ) if( !(c) ) { _printf( LogLevel_Critical, "SETUP ASSERT ( " #c " ) FAILED. ENTERING DEAD STATE.\n" ); _dead(); }
#define _FANCY_SETUP_ASSERT_OR_DEAD( c, s, d ) { YUNA.print_w( s ); _SETUP_ASSERT_OR_DEAD( c ); YUNA.print_w( " ~ok\n" ); vTaskDelay( d ); }
void setup( void ) {
    _SETUP_ASSERT_OR_DEAD( GPIO::init() == 0 );

	BITNA.blink( 9, Led_BLK, Led_RED, 50, 50 );

    Serial.begin( 115200 );
	
	_printf( LogLevel_Info, "Serial ok.\n" );
	_printf( LogLevel_Info, "I^2C bus begin..." );
	CONFIG.I2C_bus->begin();
	_printf( "ok.\n" );

    _SETUP_ASSERT_OR_DEAD( YUNA.init() == 0 );
    
    YUNA.print_w( ">/ i2c-bus >...\n" ); vTaskDelay( 300 );
    {
    static constexpr int PER_ROW = 5;
    int count = 0;
    for( int addr = 1; addr < 127; ++addr ) {
        CONFIG.I2C_bus->beginTransmission( addr );
        int status = CONFIG.I2C_bus->endTransmission();

        if( status != 0 ) continue;

        YUNA.printf_w( "0x%s%x ", ( addr < 16 ? "0" : "" ), addr ); vTaskDelay( 300 );
        if( ++count == PER_ROW ) {
            YUNA.print_w( '\n' );
            count = 0;
        }
    }
    if( count != 0 ) YUNA.print_w( '\n' );
    }

	_FANCY_SETUP_ASSERT_OR_DEAD( GRAN.init() == 0, ">/ [mpu-6050]", 300 );

    _FANCY_SETUP_ASSERT_OR_DEAD( SUSAN.init() == 0, ">/ [bmp-280]", 300 );
	
	_FANCY_SETUP_ASSERT_OR_DEAD( DYNAM.init() == 0, ">/ dynam", 300 );

    _FANCY_SETUP_ASSERT_OR_DEAD( CONFIG.init() == 0, ">/ config", 300 );

    _printf( LogLevel_Info, "Scanning for tests request.\n" );
    DYNAM.scan( Scan_Switches );
    if( DYNAM.ningning.dwn ) {
        do_tests();
    } else if( DYNAM.giselle.dwn ) {
        YUNA.print_w( "/ jmp mode ctrl." );
        CONFIG.mode.store( Mode_Ctrl );
    }

    _printf( LogLevel_Ok, "Init complete.\n" );
	BITNA.blink( 9, Led_BLK, Led_RED, 50, 50 );

	YUNA.clear().splash_logo(); vTaskDelay( 1600 );
    BITNA( Led_BLK );
}

void loop() { 
    vTaskPrioritySet( NULL, 3 );
    xSemaphoreGive( CONFIG.init_sem );
    for(;;) {
        CONFIG.mains[ CONFIG.mode ]( NULL ); 
    }
    _dead(); 
}


struct _LOOP_STATE {
    bool   _conn_rst   = true;
} LOOP_STATE;

/* 1: LOOP_STATE._conn_rst | 0: CONFIG_CTRL_MODE.wjpblu.connected() */
std::function< int( void ) >   loop_procs[]   = {
  /* 0b00 */ [] () -> bool {
    _printf( LogLevel_Info, "Disconnected from device.\n" );
    LOOP_STATE._conn_rst = true;
    _printf( LogLevel_Info, "Ready to relink...\n" );
    BITNA.blink( 10, Led_BLU, Led_RED, 50, 50 );
    return true;
  },
  /* 0b01 */ [] () -> bool {
    if( COM.loop() != 0 ) {
      BITNA.blink( 10, Led_RED, Led_BLK, 100, 500 );
      CONFIG_CTRL_MODE.wjpblu.disconnect();
      return true;
    }
    
    DYNAM.scan( Scan_All );
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



int _DYNAM::blue_tx( void ) {
  static int last_ms = millis();
  static int current_ms;

  current_ms = millis();
  if( current_ms - last_ms < CONFIG_CTRL_MODE.dynamic_period ) return 0;

  last_ms = current_ms;
  int ret = CONFIG_CTRL_MODE.wjpblu.trust_burst( this, sizeof( WJP_HEAD ) + WJP_HEAD::_dw3.sz, 0 );

  this->reset_sw_prs_rls();
  return ret;
}



/* .  .  _. . .  
|| |\/| |_| | |\ |
|| |  | | | | | \|
*/ 

static struct _FELLOW_TASK {
    static void _func_wrap( void* func ) {
        xSemaphoreTake( CONFIG.init_sem, portMAX_DELAY ); xSemaphoreGive( CONFIG.init_sem );
        ( ( void (*)( void* ) )func )( NULL );
        vTaskDelete( NULL );
    } 

    _FELLOW_TASK( const char* name, bool suspended, int stack_depth, int priority, void ( *func )( void* ) ) 
    : _func{ func }
    {
        BaseType_t status = xTaskCreate( _FELLOW_TASK::_func_wrap, name, stack_depth, ( void* )_func, priority, &_handle );
        if( status != pdPASS ) return;

        if( suspended ) this->suspend();
    }

    TaskHandle_t   _handle   = NULL;
    void           ( *_func )( void* );

    inline void suspend() { vTaskSuspend( _handle ); }
    inline void resume() { vTaskResume( _handle ); }

}

FELLOW_TASK_dynam_scan{ "dynam_scan", false, 4096, Priority_Urgent, [] ( void* ) static -> void { 
for(;;) {
    vTaskDelay( CONFIG.dyn_scan_T ); DYNAM.q_scan( Scan_All );
} } },

FELLOW_TASK_input_react{ "input_react", true, 1024, Priority_Aesth, [] ( void* ) static -> void {
    /* Given a single threshold, the joystick area around it will cause flicker. Use hysteresis to mitigate. */
    static constexpr float JS_REACT_THRESHOLD = 0.8;
    static constexpr int   REACT_COUNT        = 5;

    barra::dynamic_t          dyn;
     _DYNAM::snapshot_token_t ss_tok                = { dst: &dyn };
    int                       last_state            = 0;
    Led_                      reacts[ REACT_COUNT ] = { Led_TRQ, Led_GRN, Led_YLW, Led_PRP, Led_BLU };
    int                       react_at              = -1;

for(;;) {
    DYNAM.snapshot( &ss_tok );

    int crt_state = 0;

    for( auto& sw : dyn.switches )
        ( crt_state |= sw.dwn ) <<= 1;

    for( auto& js : dyn.joysticks ) { 
        ( crt_state |=  js.sw.dwn ) <<= 1;
        ( crt_state |= ( abs( js.x ) >= JS_REACT_THRESHOLD ) ) <<= 1;
        ( crt_state |= ( abs( js.y ) >= JS_REACT_THRESHOLD ) ) <<= 1;
    }

    if( crt_state != 0 && last_state != crt_state ) 
        BITNA( reacts[ ++react_at %= REACT_COUNT ] );
    else if( crt_state == 0 ) 
        BITNA( Led_BLK );

    last_state = crt_state;
} } };


int _CONFIG::init( void ) {
	int status = 0;

    mains[ Mode_Brdg ] = &main_brdg;
    mains[ Mode_Game ] = &main_game;
    mains[ Mode_Ctrl ] = &main_ctrl;

    status = CONFIG_BRDG_MODE.init(); if( status != 0 ) return status;
	status = CONFIG_GAME_MODE.init(); if( status != 0 ) return status;
	status = CONFIG_CTRL_MODE.init(); if( status != 0 ) return status;

	return 0;
}



/* ._  ._  .   .__
|| |_\ |_| |\  | ._
|| |_| | \ |_\ |__|
*/ 
int _CONFIG_BRDG_MODE::init( void ) {
    return 0;
}

void main_brdg( void* arg ) {
    FELLOW_TASK_dynam_scan.resume();
    FELLOW_TASK_input_react.resume();

    MAIN_LOOP_ON( Mode_Brdg ) {
        vTaskDelay( 1000 );
    }
}



/* .__   _  .  . .__
|| | ._ |_| |\/| |_
|| |__| | | |  | |__
*/ 
int _CONFIG_GAME_MODE::init( void ) {
	return 0;
}

void main_game( void* arg ) {

}



/* .__ __.__ ._  .
|| |     |   |_| |
|| |__   |   | \ |__
*/ 
static WJP_QGSTBL_ENTRY wjp_QGSTBL[ 3 ] = {
{ 
	str_id: "BITNA_CRT", 
	sz: 1, 
	WJP_QGSTBL_READ_ONLY, 
	qset_func: NULL,
	qget_func: NULL, 
	src: &BITNA._crt
},
{ 
	str_id: "GRAN_ACC_RANGE", 
	sz: 1, 
	WJP_QGSTBL_READ_WRITE, 
	qset_func: [] WJP_QSET_LAMBDA { return GRAN.set_acc_range( ( MPU9250_accRange )*( uint8_t* )args.addr ) ? NULL : "GRAN_ACC_INVALID_RANGE"; },
	qget_func: NULL,
	src: NULL
},
{ 
	str_id: "GRAN_GYR_RANGE", 
	sz: 1, 
	WJP_QGSTBL_READ_WRITE, 
	qset_func: [] WJP_QSET_LAMBDA { return GRAN.set_gyr_range( ( MPU9250_gyroRange )*( uint8_t* )args.addr ) ? NULL : "GRAN_GYR_INVALID_RANGE"; },
	qget_func: NULL,
	src: NULL
}
};

int _CONFIG_CTRL_MODE::init( void ) {
	wjpblu.bind_qgstbl( WJP_QGSTBL{ 
      	entries: wjp_QGSTBL, 
      	count: sizeof( wjp_QGSTBL ) / sizeof( WJP_QGSTBL_ENTRY ) 
    } );

    return wjpblu.init( 0 );
}

void main_ctrl( void* arg ) {
    FELLOW_TASK_dynam_scan.suspend();

    CONFIG_CTRL_MODE.wjpblu.begin( barra::DEVICE_NAME );    

    MAIN_LOOP_ON( Mode_Ctrl ) {
        loop_procs[ ( LOOP_STATE._conn_rst << 1 ) | CONFIG_CTRL_MODE.wjpblu.connected() ]();
    } 

    CONFIG_CTRL_MODE.wjpblu.end();
}
