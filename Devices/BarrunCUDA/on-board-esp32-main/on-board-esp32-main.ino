/*====== BarraCUDA - Main - Vatca "Mipsan" Tudor-Horatiu 
|
|=== DESCRIPTION
> Ask Daniel. ( massive red flag )
> Yes, everything in one file, deal w/ it.
|
======*/
#define BARRUNCUDA_WJP_VER 3

#include "../barracuda.hpp"
#include "esp_task_wdt.h"

#include <Wire.h> /* I2C bus. */

#include <MPU6050_WE.h> /* Gyro. */

#define SSD1306_NO_SPLASH
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> /* Display. */
#include <Adafruit_BME280.h> /* Temperature/Pressure/Humidity/Altitude. */
#include <Adafruit_seesaw.h> /* Digital GPIO extender. */

#include <DFRobot_BMM150.h> /* Magnetometer. */

#include <RCWL_1X05.h> /* Sonar. */

#include "../../IXN/common_utils.hpp"
#if BARRUNCUDA_WJP_VER == 1
    #define WJP_ARCHITECTURE_LITTLE
    #define WJP_ENVIRONMENT_RTOS
    #include "../../IXN/Driver/wjp_on_bths.hpp"
#endif
using namespace ixN::uC;

#if BARRUNCUDA_WJP_VER == 3
    #include "../../runik-patriot/abstraction_layer.hpp"
#endif

#include "graphics.hpp"

#include <atomic>


#define uC_CORE_0 0
#define uC_CORE_1 1

#define uC_PWM_CHANNEL_BITNA_R   0
#define uC_PWM_CHANNEL_BITNA_G   1
#define uC_PWM_CHANNEL_BITNA_B   2
#define uC_PWM_CHANNEL__UNUSED_3 3
#define uC_PWM_CHANNEL_MIRU      4

typedef   int8_t   GPIO_pin_t;

enum Mode_ : int {
    Mode_Brdg = 0,
    Mode_Game = 1,
    Mode_Ctrl = 2,

    _Mode_Count
};

enum Priority_ : int {
    Priority_Aesth   = 0,
    Priority_SubMain = 2,
    Priority_Main    = 3,
    Priority_AbvMain = 4,
    Priority_Urgent  = 5
};



void main_brdg( void* );
void main_game( void* );
void main_ctrl( void* );

struct _BRIDGE;



#define MAIN_LOOP_ON( _M ) for(; CONFIG.mode.load( std::memory_order_relaxed ) == _M ;)
static struct _CONFIG {
	int init( void );

    void ( *mains[ _Mode_Count ] )( void* );

    std::atomic_int     mode                  = { Mode_Brdg };

    TwoWire*            I2C_bus               = &Wire;

    const int           I2C_addr_YUNA         = 0x3C;
    const int           I2C_addr_GRAN         = 0x68;
    const int           I2C_addr_SUSAN        = 0x77;
    const int           I2C_addr_CHIM         = 0x13;
    const int           I2C_addr_GOLANI       = 0x57;
    const int           I2C_addr_GPIO_DEX_0   = 0x49;

    SemaphoreHandle_t   init_sem              = { xSemaphoreCreateBinary() };

    int                 dyn_scan_T            = 20;
    int                 spot_T                = 33;

    void*               _arg                  = NULL;

    Mode_ xchg_mode( Mode_ m, void* arg ) { _arg = arg; return ( Mode_ )mode.exchange( m, std::memory_order_seq_cst ); }

} CONFIG;

static struct _CONFIG_BRDG_MODE {
    int init( void );

    inline static constexpr int   STRIDE   = 8;

    _BRIDGE*   _root   = NULL;
    _BRIDGE*   _home   = NULL;
    
    struct { 
        _BRIDGE*   crt           = NULL;
        int        dep           = 0; 
        char       path[ 256 ]   = { ">//" };

        char* _path_rss( int idx ) {
            int len = strlen( path );
            char* ptr = path + len - 1;

            if( *ptr == '/' ) --ptr;

            while( ptr >= path ) {
                if( *ptr == '/' && --idx == 0 ) return ptr;
                --ptr;
            }

            return path + 1;
        }

        void _path_mks() {
            char* ptr = path + strlen( path ) + 1;
            *ptr = '\0';
            *--ptr = '/';
        }

        void path_x( const char* name, bool has_subs ) {
            strcpy( this->_path_rss( 1 ) + 1, name );
            if( has_subs ) this->_path_mks();
        }

        void path_d( const char* name, bool has_subs ) {
            strcpy( path + strlen( path ), name );
            if( has_subs ) this->_path_mks();
        }

        void path_s() {
            *( this->_path_rss( 1 ) + 1 ) = '\0';
        }
    } _nav;

    void bridge_back( void );
    int bridge_N( void );
    int bridge_S( void );
    int bridge_E( void );
    int bridge_W( void );

} CONFIG_BRDG_MODE;

static struct _CONFIG_GAME_MODE {
	int init( void );

} CONFIG_GAME_MODE;

static struct _CONFIG_CTRL_MODE {
	int init( void );

#if BARRUNCUDA_WJP_VER == 1
	WJP_on_BluetoothSerial   wjpblu      = {};
    bool                     _conn_rst   = true;
#endif
#if BARRUNCUDA_WJP_VER == 3
#endif

} CONFIG_CTRL_MODE;



struct GPIO { 
    inline static const int     GND        = 0;
    inline static const int     VCC        = 1;
    inline static const int     ADC_nmax   = 4095;
    inline static const float   ADC_fmax   = 4095.0f;
    inline static const int     DC_n100    = 255;
    inline static const int     DC_n0      = 0;

    inline static struct { const GPIO_pin_t sw, x, y; } rachel{ sw: 27, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
    /*                                                  |lower left                     |upper right */

    inline static const GPIO_pin_t giselle = 5, karina = 18, ningning = 23, winter = 19;
    /*                             |blue        |red         |yellow        |green */

    inline static const GPIO_pin_t naksu = 15;
    /*                             |light */

    inline static const GPIO_pin_t tanya = 4;
    /*                             |potentiometer */

    inline static const GPIO_pin_t miru = 16;
    /*                             |buzzer */

    inline static const GPIO_pin_t minju = 34;
    /*                             | analog multiplexer */

    inline static const GPIO_pin_t xabara = 17;
    /*                             |bridge */

    inline static struct { const GPIO_pin_t r, g, b, x; } bitna{ r: 13, g: 14, b: 12, x: LED_BUILTIN };
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
        pinMode( tanya, INPUT );

        pinMode( miru, INPUT );

        pinMode( minju, INPUT );

        pinMode( xabara, INPUT_PULLUP );
    
        ledcAttachChannel( bitna.r, 100, 8, uC_PWM_CHANNEL_BITNA_R );
        ledcAttachChannel( bitna.g, 100, 8, uC_PWM_CHANNEL_BITNA_G );
        ledcAttachChannel( bitna.b, 100, 8, uC_PWM_CHANNEL_BITNA_B );
        pinMode( bitna.x, OUTPUT );

        _printf( "ok.\n" );

        return 0;
    }

    inline static int  D_R ( GPIO_pin_t pin )               { return digitalRead( pin ); }
    inline static void D_W ( GPIO_pin_t pin, int level )    { digitalWrite( pin, level ); }
    inline static int  A_R ( GPIO_pin_t pin )               { return analogRead( pin ); }

    inline static void make_HI( GPIO_pin_t pin ) { pinMode( pin, INPUT ); }

    /**
     * @brief Performs N analog reads on pin.
     * @returns The mean of the `N` reads, as `_T`, in the range [ `0,  2^{ADC resolution}-1` ].
     * @param[ in ] pin: The pin number.
     * @param[ in ] N: The number of reads to perform.
     */
    template< typename _T > _T static A_Rm( GPIO_pin_t pin, int N ) {
        int acc = 0;
        for( int n = 1; n <= N; ++n ) acc += A_R( pin );
        return ( _T )acc / N;
    }

};

struct GPIO_DEX_0 {
    inline static Adafruit_seesaw    _seesaw   = { CONFIG.I2C_bus };

    inline static const GPIO_pin_t  suzyq  = 16;
    /*                              |laser */

    static int init( void ) {
        if( !_seesaw.begin( CONFIG.I2C_addr_GPIO_DEX_0 ) ) return -1;

        _seesaw.pinMode( suzyq, OUTPUT );

        return 0;
    }

    inline static int  D_R ( GPIO_pin_t pin )            { return _seesaw.digitalRead( pin ); }
    inline static void D_W ( GPIO_pin_t pin, int level ) { _seesaw.digitalWrite( pin, level ); }
    inline static int  A_R ( GPIO_pin_t pin )            { return _seesaw.analogRead( pin ); }
};



#define Is_ARMABLE__ARM_OVR virtual int _arm( void* arg ) override
#define Is_ARMABLE__DISARM_OVR virtual int _disarm( void* arg ) override
struct Is_ARMABLE {
    std::atomic_bool   _armed   = { false };

    inline bool is_armed( std::memory_order mo = std::memory_order_seq_cst ) {
        return _armed.load( mo );
    }

    virtual int _arm( void* arg ) = 0;
    virtual int _disarm( void* arg ) = 0;

    inline int arm( void* arg ) {
        bool was_armed = false;
        if( !_armed.compare_exchange_strong( was_armed, true, std::memory_order_seq_cst ) ) return 0;

        return this->_arm( arg );
    }

    inline int disarm( void* arg ) {
        bool was_armed = true;
        if( !_armed.compare_exchange_strong( was_armed, false, std::memory_order_seq_cst ) ) return 0;

        return this->_disarm( arg );
    }
};



#define Led_BLK 0x00'00'00
#define Led_RED 0xFF'00'00
#define Led_GRN 0x00'FF'00
#define Led_BLU 0x00'00'FF
#define Led_YLW (Led_RED|Led_GRN)
#define Led_TRQ (Led_BLU|Led_GRN)
#define Led_PRP (Led_RED|Led_BLU)
#define Led_WHT (Led_RED|Led_GRN|Led_BLU)
#define Led_ORANGE (LED_MAKE_RGB( 255, 63, 0 ))
#define Led_PURPLE (LED_MAKE_RGB( 60, 0, 255))

#define LED_CH_R( rgb ) ( ( rgb >> 16 ) & 0xFF )
#define LED_CH_G( rgb ) ( ( rgb >> 8 ) & 0xFF )
#define LED_CH_B( rgb ) ( rgb & 0xFF )
#define LED_CH( ch, rgb ) ( ( rgb >> ((2-ch)*8) ) & 0xFF )

#define LED_MAKE_RGB( r, g, b ) ( ( r << 16 ) | ( g << 8 ) | b )
#define LED_MAKE_CH( ch, val ) ( ( val & 0xFF ) << ((2-ch)*8) )

struct Led_ {
    Led_() : rgb{ 0 } {}
    Led_( int rgb ) : rgb{ rgb } {}
    Led_( int r, int g, int b ) : r{ r }, g{ g }, b{ b } {}

    union {
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t _unused;
        };
        int rgb;
    };

    operator int () { return rgb; }
};

static struct _BITNA {
    inline void x_set( void ) { 
        GPIO::D_W( GPIO::bitna.x, GPIO::VCC );
    }

    inline void x_reset( void ) {
        GPIO::D_W( GPIO::bitna.x, GPIO::GND );
    }

    void x_blink( int N, int ms_on, int ms_off, bool keep_on, bool strip ) {
        for( int n = 1; n <= N; ++n ) {
            this->x_set(); if( strip ) this->make( Led_BLU ); vTaskDelay( ms_on ); 
            this->x_reset(); if( strip ) this->make( Led_BLK ); vTaskDelay( ms_off );
        }
        if( keep_on ) this->x_set();
    }

    Led_   _crt   = Led_BLK;
    Led_   _tar   = Led_BLK;

    inline void _set( Led_ led ) {
        ledcWrite( GPIO::bitna.r, 255 - LED_CH_R( led.rgb ) );
        ledcWrite( GPIO::bitna.g, 255 - LED_CH_G( led.rgb ) );
        ledcWrite( GPIO::bitna.b, 255 - LED_CH_B( led.rgb ) );
    }
    
    inline _BITNA& target( Led_ led ) {
        _tar = led;
        return *this;
    }

    inline _BITNA& make( Led_ led ) {
        this->_set( _crt = led );
        return *this;
    }

    inline _BITNA& jump( Led_ led ) {
        this->_set( _tar = _crt = led );
        return *this;
    }
    
    inline _BITNA& operator () ( Led_ led ) { return this->jump( led ); }

    /**
     * @brief OR with the current color and write it.
     */
    inline _BITNA& operator |= ( Led_ led ) { return this->jump( _crt.rgb | led.rgb ); }
    /**
     * @brief AND with the current color and write it.
     */
    inline _BITNA& operator &= ( Led_ led ) { return this->jump( _crt.rgb & led.rgb ); }
    /**
     * @brief XOR with the current color and write it.
     */
    inline _BITNA& operator ^= ( Led_ led ) { return this->jump( _crt.rgb ^ led.rgb ); }

    /**
     * @brief Blink the LED.
     * @attention The LED color is left as `off_rgb` after this function returns.
     * @param[ in ] N: The number of cycles.
     * @param[ in ] on/off_rgb: The RGB during the `ON/OFF` period. 
     * @param[ in ] ms_on/off: The `ON/OFF` period duration, in `ms`.
     */
    _BITNA& blink( int N, Led_ on_led, Led_ off_led, int ms_on, int ms_off ) {
        for( int n = 1; n <= N; ++n ) {
            this->jump( on_led ); vTaskDelay( ms_on ); this->jump( off_led ); vTaskDelay( ms_off );
        }
        return *this;
    }

} BITNA;


enum YunaAV_ {
    YunaAV_C, YunaAV_TL, YunaAV_TR, YunaAV_BL, YunaAV_BR
};

static struct _YUNA : public Adafruit_SSD1306 {
    using Adafruit_SSD1306::Adafruit_SSD1306;
    using Adafruit_SSD1306::WIDTH;
    using Adafruit_SSD1306::HEIGHT;
    
    /**
     * @brief Configures the display.
     * @returns `0` on success, negative otherwise.
     */
    int init( void ) {
        if( !this->begin( SSD1306_SWITCHCAPVCC, CONFIG.I2C_addr_YUNA ) ) return -1;
        this->setFont( &BARRA_GFX_FONT );
        this->setTextColor( SSD1306_WHITE );
        this->setCursor( 0, 0 );
        this->setTextWrap( false );
		this->setTextSize( 1 );
        this->clearDisplay();

        //Adafruit_SSD1306::buffer = Adafruit_SSD1306::buffer;
        
        return 0;
    }

	inline _YUNA& clear_w( void ) { this->clearDisplay(); this->display(); return *this; }

	template< typename _T > inline _YUNA& print_w( _T&& arg ) { this->print( std::forward< _T >( arg ) ); this->display(); return *this; }

    template< typename ..._Args >
    inline _YUNA& printf_w( const char* fmt, _Args&&... args ) {
        static constexpr int BUF_MAX_SIZE = 256;
        char buffer[ BUF_MAX_SIZE ];
        sprintf( buffer, fmt, std::forward< _Args >( args )... );
        return this->print_w( buffer );
    }

    template< typename ..._Args >
    _YUNA& printf_av( YunaAV_ av, int16_t x, int16_t y, const char* fmt, _Args&&... args ) {
        int16_t cx, cy;
        uint16_t w, h;

        char buffer[ 128 ];
        sprintf( buffer, fmt, std::forward< _Args >( args )... );

        this->getTextBounds( buffer, x, y, &cx, &cy, &w, &h );

        switch( av ) {
            case YunaAV_C:  this->setCursor( cx - w/2, cy - h/2 ); break;
            case YunaAV_TL: this->setCursor( cx, cy ); break;          
            case YunaAV_TR: this->setCursor( cx - w + 1, cy ); break;
            case YunaAV_BL: this->setCursor( cx, cy - h + 1 ); break;
            case YunaAV_BR: this->setCursor( cx - w + 1, cy - h + 1 ); break;
        }

        this->print( buffer );
        
        return *this;
    }

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

} GRAN{ CONFIG.I2C_bus, CONFIG.I2C_addr_GRAN };


static struct _NAKSU {
    enum Unit_ : int {
        Unit_Raw, Unit_ELA, Unit_Lux, UNIT_COUNT
    };

    inline static Unit_   _unit   = Unit_ELA;

    static float from_raw( float raw ) {
        switch( _unit ) {
            case Unit_Raw: return raw;
            case Unit_ELA: return 1.0 - sqrt( raw / GPIO::ADC_fmax );
            case Unit_Lux: raw /= ( GPIO::ADC_fmax / 100.0 ); return pow( 10.0, -0.164 * ( raw - 12.0 ) ) + 10.0 - raw / 10.0;
        }
        return -1.0;
    }

    inline static int set_unit( Unit_ u ) {
        if( u < 0 || u >= UNIT_COUNT ) return -1;
        _unit = u;
        return 0;   
    }

} NAKSU;


static struct _SUSAN : public Adafruit_BME280 {
    int init( void ) {
        if( !this->Adafruit_BME280::begin( CONFIG.I2C_addr_SUSAN, CONFIG.I2C_bus  ) ) return -1;
        return 0;
    }

} SUSAN{};


static struct _CHIM : public DFRobot_BMM150_I2C {
    using DFRobot_BMM150_I2C::DFRobot_BMM150_I2C;

    int init( void ) {
        if( this->DFRobot_BMM150_I2C::begin() != 0 ) return -1;

        this->DFRobot_BMM150_I2C::setOperationMode( BMM150_POWERMODE_NORMAL );
        this->DFRobot_BMM150_I2C::setPresetMode( BMM150_PRESETMODE_HIGHACCURACY );
        this->DFRobot_BMM150_I2C::setRate( BMM150_DATA_RATE_10HZ );
        this->DFRobot_BMM150_I2C::setMeasurementXYZ( MEASUREMENT_X_ENABLE, MEASUREMENT_Y_ENABLE, MEASUREMENT_Z_DISABLE );

        return 0;
    }

    sBmm150MagData_t read( void ) {
        return this->DFRobot_BMM150_I2C::getGeomagneticData();
    }

    float degs( void ) {
        return this->DFRobot_BMM150_I2C::getCompassDegree();
    }

} CHIM{ CONFIG.I2C_bus, CONFIG.I2C_addr_CHIM };


static struct _MIRU : Is_ARMABLE {
    struct _wave_frag_t {
        int   freq   = 0;
        int   ms     = 0;
    };

    struct _wave_desc_t {
        const _wave_frag_t*   frags        = NULL;
        int                   frag_count   = 0;
    };

    int init( void ) {
        _wave_q = xQueueCreate( 1, sizeof( _wave_desc_t ) );
        return 0;
    }

    QueueHandle_t      _wave_q   = NULL;

    inline int q_push( const _wave_desc_t* desc ) {
        return xQueueSend( _wave_q, desc, 0 ) == pdPASS ? 0 : -1;
    }

    inline int q_push( const _wave_desc_t& desc ) {
        return this->q_push( &desc );
    }

    inline _MIRU& write( int freq, int ms ) {
        this->arm( &freq );
        vTaskDelay( ms );
        this->disarm( NULL );
        return *this;
    }

    inline _MIRU& write( int freq, int ms_on, int N, int ms_off ) {
        int n = 1;

        this->arm( &freq );

    l_loop:
        vTaskDelay( ms_on );
        if( n >= N ) goto l_end;
        ++n;

        this->flat();
        vTaskDelay( ms_off );
        this->pwm();
        goto l_loop;

    l_end:
        this->disarm( NULL );
        return *this;
    }

    inline _MIRU& pwm( int dc = 127 ) {
        ledcWrite( GPIO::miru, dc );
        return *this;
    }

    inline _MIRU& flat( void ) {
        return this->pwm( 255 );
    }

    Is_ARMABLE__ARM_OVR{
        int freq = *( int* )arg;

        ledcAttachChannel( GPIO::miru, 10, 8, uC_PWM_CHANNEL_MIRU );
        this->make( freq );
        this->pwm( freq > 0 ? 127 : 255 );
        return 0;
    }

    Is_ARMABLE__DISARM_OVR{
        this->flat();
        ledcDetach( GPIO::miru );
        pinMode( GPIO::miru, INPUT );
        return 0;
    }

    inline _MIRU& make( int freq ) {
        ledcWriteTone( GPIO::miru, freq );
        return *this;
    }

} MIRU;


static struct _SUZYQ : Is_ARMABLE {
    int init( void ) {
        return 0;
    }

    Is_ARMABLE__ARM_OVR{
        GPIO_DEX_0::D_W( GPIO_DEX_0::suzyq, GPIO::VCC );
        return 0;
    }

    Is_ARMABLE__DISARM_OVR{
        GPIO_DEX_0::D_W( GPIO_DEX_0::suzyq, GPIO::GND );
        return 0;
    }

} SUZYQ;


static struct _GOLANI : RCWL_1X05 {
    using RCWL_1X05::RCWL_1X05;

    int init( void ) {
        if( !this->begin( CONFIG.I2C_bus, CONFIG.I2C_addr_GOLANI ) ) return -1;

        this->setMode( RCWL_1X05::oneShot );

        return 0;
    }
} GOLANI;



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
static struct _DYNAM : barra::dynamic_t {
    struct {
        struct { float x, y; } rachel, samantha;
    } _idle_reads;

    struct snapshot_token_t {
        barra::dynamic_t*   dst          = NULL;
        bool                blk          = true;
        int                 _seq         = 0;
        bool                _fs          = true;
        uint8_t             _xjtt[ 2 ]   = { HIGH, HIGH };
        uint8_t             _yjtt[ 2 ]   = { HIGH, HIGH };
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

        _printf( LogLevel_Info, "Joysticks calibrate... " );
        _idle_reads.rachel.x = GPIO::A_Rm< float >( GPIO::rachel.x, 10 ); _idle_reads.rachel.y = GPIO::A_Rm< float >( GPIO::rachel.y, 10 );
        _idle_reads.samantha.x = GPIO::A_Rm< float >( GPIO::samantha.x, 10 ); _idle_reads.samantha.y = GPIO::A_Rm< float >( GPIO::samantha.y, 10 );
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

            tar->rachel.x = GPIO::A_R( GPIO::rachel.x ); tar->rachel.y = GPIO::A_R( GPIO::rachel.y );
            tar->samantha.x = GPIO::A_R( GPIO::samantha.x ); tar->samantha.y = GPIO::A_R( GPIO::samantha.y );

            _resolve_joystick_axis( tar->rachel.x, _idle_reads.rachel.x ); _resolve_joystick_axis( tar->rachel.y, _idle_reads.rachel.y );
            _resolve_joystick_axis( tar->samantha.x, _idle_reads.samantha.x ); _resolve_joystick_axis( tar->samantha.y, _idle_reads.samantha.y );

            tar->samantha.y *= -1.0f;
        }
        /* Switches */ l_switches: _DYNAM_SCAN_IF( 1, l_accel ); {
            const auto _resolve_switch = [] ( barra::switch_t& sw, GPIO_pin_t pin ) static -> void {
                int is_dwn = !GPIO::D_R( pin );
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
            tar->naksu.lvl = _NAKSU::from_raw( GPIO::A_R( GPIO::naksu ) );
        }
        /* Potentiometer */ l_potentio: _DYNAM_SCAN_IF( 5, l_end ); {
            tar->tanya.lvl = analogRead( GPIO::tanya ) / GPIO::ADC_fmax;
        }
    l_end:
        return;
    }
    
    /**
     * @brief Updates the dynam values.
     * @param[ in ] flags: Flags to select function behaviour. Default, updates everything.
     */
    inline _DYNAM& scan( int flags ) {
        this->_scan( flags, &( barra::dynamic_t& )*this );
        return *this;
    }

    inline void q_scan( int flags ) {
        this->_scan( flags, &( barra::dynamic_t& )*this );
    
        xQueueReset( _q[ _seq ^ 1 ].handle );
        xQueueOverwrite( _q[ _seq ].handle, &( barra::dynamic_t& )*this );

        _seq ^= 1;
    }

    int snapshot( snapshot_token_t* tok ) {
        barra::switch_t sws[ 6 ] = {
            tok->dst->samantha.sw, tok->dst->rachel.sw, 
            tok->dst->giselle, tok->dst->karina, tok->dst->ningning, tok->dst->winter
        };

        if( xQueuePeek( _q[ tok->_seq ].handle, tok->dst, tok->blk ? portMAX_DELAY : 0 ) == errQUEUE_EMPTY ) return -1;
        tok->_seq ^= 1;

        if( tok->_fs == true ) {
            tok->_fs = false;
            return 0;
        }

        _DYNAM::_resolve_cmp_js( tok->_xjtt[ 0 ], tok->_yjtt[ 0 ], tok->dst->samantha );
        _DYNAM::_resolve_cmp_js( tok->_xjtt[ 1 ], tok->_yjtt[ 1 ], tok->dst->rachel );

        _DYNAM::_resolve_cmp_sw( sws[ 0 ], tok->dst->samantha.sw );
        _DYNAM::_resolve_cmp_sw( sws[ 1 ], tok->dst->rachel.sw ); 
        _DYNAM::_resolve_cmp_sw( sws[ 2 ], tok->dst->giselle ); 
        _DYNAM::_resolve_cmp_sw( sws[ 3 ], tok->dst->karina ); 
        _DYNAM::_resolve_cmp_sw( sws[ 4 ], tok->dst->ningning ); 
        _DYNAM::_resolve_cmp_sw( sws[ 5 ], tok->dst->winter );

        return 0;
    }

    barra::dynamic_t snapshot_once( void ) {
        barra::dynamic_t    buffer;
        snapshot_token_t    token    = { dst: &buffer, blk: true };

        this->snapshot( &token );
        return buffer; 
    }

    inline static void _resolve_cmp_sw( barra::switch_t& old, barra::switch_t& crt ) {
        switch( ( old.dwn << 1 ) | crt.dwn ) {
            case 0b00: [[fallthrough]];
            case 0b11: crt.rls = 0; crt.prs = 0; break;
            case 0b01: crt.rls = 0; crt.prs = 1; break;
            case 0b10: crt.rls = 1; crt.prs = 0; break;
        }
    }

#define _DYNAM_RESOLVE_CMP_JS_AXIS( jtt, axis ) \
if( jtt == HIGH && ( js.axis >= barra::JS_HYST_HIGH || js.axis <= -barra::JS_HYST_HIGH ) ) { jtt = LOW; js.edg.axis = js.axis >= barra::JS_HYST_HIGH ? 1 : -1; is##axis = 1; } \
else if( jtt == LOW && abs( js.axis ) <= barra::JS_HYST_LOW ) { jtt = HIGH; js.edg.axis = 0; is##axis = 1; } \
else { js.edg.axis = is##axis = 0; }
    inline static void _resolve_cmp_js( uint8_t& xjtt, uint8_t& yjtt, barra::joystick_t& js ) {
        bool isx = false, isy = false;
        _DYNAM_RESOLVE_CMP_JS_AXIS( xjtt, x );
        _DYNAM_RESOLVE_CMP_JS_AXIS( yjtt, y );
        js.edg.is = isx | isy;
    }

} DYNAM;



void _dead( void ) {
  portDISABLE_INTERRUPTS();
  vTaskSuspendAll();
#if BARRUNCUDA_WJP_VER == 1
  CONFIG_CTRL_MODE.wjpblu.end();
#endif
  Wire.end();
  while( 1 ) BITNA.blink( 1, Led_RED, Led_BLK, 1000, 1000 );
}



#define _SETUP_ASSERT_OR_DEAD( c ) if( !(c) ) { _printf( LogLevel_Critical, "SETUP ASSERT ( " #c " ) FAILED. ENTERING DEAD STATE.\n" ); _dead(); }
#define _FANCY_SETUP_ASSERT_OR_DEAD( c, s ) { YUNA.print_w( s ); _SETUP_ASSERT_OR_DEAD( c ); YUNA.print_w( " ~ok\n" ); vTaskDelay( FANCY_DELAY_MS ); }
void setup( void ) {
    vTaskDelay( 2000 );
    vTaskPrioritySet( NULL, Priority_Main );

    static constexpr int FANCY_DELAY_MS = 100;

    _SETUP_ASSERT_OR_DEAD( GPIO::init() == 0 );

	BITNA.blink( 9, Led_BLK, Led_RED, 50, 50 );

    Serial.begin( 115200 );
	
	_printf( LogLevel_Info, "Serial ok.\n" );
	_printf( LogLevel_Info, "I^2C bus begin..." );
	CONFIG.I2C_bus->begin();
	_printf( "ok.\n" );

    _SETUP_ASSERT_OR_DEAD( MIRU.init() == 0 );
    MIRU.write( 440, 100 );

    _SETUP_ASSERT_OR_DEAD( YUNA.init() == 0 );
    
    YUNA.print_w( ">/ i2c-bus >...\n" ); vTaskDelay( FANCY_DELAY_MS );
    {
    static constexpr int PER_ROW = 5;
    int count = 0;
    for( int addr = 1; addr < 127; ++addr ) {
        CONFIG.I2C_bus->beginTransmission( addr );
        int status = CONFIG.I2C_bus->endTransmission();

        if( status != 0 ) continue;

        YUNA.printf_w( "0x%s%x ", ( addr < 16 ? "0" : "" ), addr ); vTaskDelay( FANCY_DELAY_MS );
        if( ++count == PER_ROW ) {
            YUNA.print_w( '\n' );
            count = 0;
        }
    }
    if( count != 0 ) YUNA.print_w( '\n' );
    }

    MIRU.write( 440, 100 );
    YUNA.setCursor( 0, 0 ); YUNA.clearDisplay();
    YUNA.print_w( ">/ hardware >...\n" ); vTaskDelay( FANCY_DELAY_MS );

	_FANCY_SETUP_ASSERT_OR_DEAD( GRAN.init() == 0, ">/ [mpu-6050]" );
    _FANCY_SETUP_ASSERT_OR_DEAD( SUSAN.init() == 0, ">/ [bme-280]" );
    _FANCY_SETUP_ASSERT_OR_DEAD( CHIM.init() == 0, ">/ [bmm-150]" );

    MIRU.write( 440, 100 );
    YUNA.setCursor( 0, 0 ); YUNA.clearDisplay();
    YUNA.print_w( ">/ hardware >...\n" ); vTaskDelay( FANCY_DELAY_MS );

    _FANCY_SETUP_ASSERT_OR_DEAD( GPIO_DEX_0::init() == 0, ">/ [tiny-416]" );
    _FANCY_SETUP_ASSERT_OR_DEAD( SUZYQ.init() == 0, ">/ [kyp-008]" );
    //_FANCY_SETUP_ASSERT_OR_DEAD( GOLANI.init() == 0, ">/ [hc-sr04]" );

    MIRU.write( 440, 100 );
    YUNA.setCursor( 0, 0 ); YUNA.clearDisplay();
    YUNA.print_w( ">/ software >...\n" ); vTaskDelay( FANCY_DELAY_MS );
	
	_FANCY_SETUP_ASSERT_OR_DEAD( DYNAM.init() == 0, ">/ dynam" );

    _FANCY_SETUP_ASSERT_OR_DEAD( CONFIG.init() == 0, ">/ config" );

    srand( GPIO::A_R( GPIO::samantha.x ) + GPIO::A_R( GPIO::rachel.x ) );

    YUNA.setCursor( 0, 0 ); YUNA.clearDisplay();
    BITNA.make( Led_ORANGE );
    YUNA.print_w( ">/ boot int... " );
    MIRU.write( 80, 10, 10, 100 );

    if( GPIO::D_R( GPIO::xabara ) == GPIO::GND ) {
        YUNA.print_w( "yes\n");

        while( GPIO::D_R( GPIO::xabara ) == GPIO::GND ) { 
            BITNA.blink( 1, Led_BLK, Led_ORANGE, 50, 200 ); 
        }

        DYNAM.scan( Scan_Switches );

        if( DYNAM.giselle.dwn ) {
            YUNA.print_w( "/> jmp mode ctrl\n" );
            CONFIG.mode.store( Mode_Ctrl );
        }

    } else {
        YUNA.print_w( "no\n");
    }

    _printf( LogLevel_Ok, "Init complete.\n" );

    YUNA.print_w( ">/ boot ok" );
    MIRU.write( 440, 100, 3, 100 );
	BITNA.blink( 3, Led_TRQ, Led_BLK, 50, 50 );
}



/* .  .  _. . .  
|| |\/| |_| | |\ |
|| |  | | | | | \|
*/ 
enum FellowTask_ : int {
    FellowTask_DynamScan      = ( 1 << 0 ),
    FellowTask_InputReact     = ( 1 << 1 ),
    FellowTask_LedController  = ( 1 << 2 ),
    FellowTask_WaveController = ( 1 << 3 )
};
static struct _FELLOW_TASK {
    static void _func_wrap( void* func ) {
        xSemaphoreTake( CONFIG.init_sem, portMAX_DELAY ); xSemaphoreGive( CONFIG.init_sem );
        ( ( void (*)( void* ) )func )( NULL );
        vTaskDelete( NULL );
    } 

    _FELLOW_TASK( const char* name, int stack_depth, int priority, void ( *func )( void* ) ) 
    : _func{ func }
    {
        BaseType_t status = xTaskCreate( _FELLOW_TASK::_func_wrap, name, stack_depth, ( void* )_func, priority, &_handle );
        static int _array_idx = 0; _FELLOW_TASK::_array[ _array_idx++ ] = this;
        if( status != pdPASS ) return;
    }

    inline static constexpr int   _ARR_SZ   = 16;
    inline static _FELLOW_TASK*   _array[ _ARR_SZ ];

    TaskHandle_t   _handle   = NULL;
    void           ( *_func )( void* );

    inline void suspend() { vTaskSuspend( _handle ); }
    inline void resume() { vTaskResume( _handle ); }

    inline static void require( int tasks ) {
        for( int idx = 0; idx < _ARR_SZ && _array[ idx ] != NULL; ++idx ) {
            if( ( tasks >> idx ) & 1 ) _array[ idx ]->resume(); else _array[ idx ]->suspend();
        }
    }
}

FELLOW_TASK_dynam_scan{ "dynam_scan", 4096, Priority_Urgent, [] ( void* ) static -> void { 
for(;;) {
    vTaskDelay( CONFIG.dyn_scan_T ); DYNAM.q_scan( Scan_All );
} } },

FELLOW_TASK_input_react{ "input_react", 1024, Priority_Aesth, [] ( void* ) static -> void {
    static constexpr int   REACT_COUNT   = 5;

    barra::dynamic_t          dyn;
     _DYNAM::snapshot_token_t ss_tok                = { dst: &dyn, blk: true };
    int                       last_state            = 0;
    Led_                      reacts[ REACT_COUNT ] = { Led_TRQ, Led_GRN, Led_ORANGE, Led_PRP, Led_BLU };
    int                       react_at              = -1;

for(;;) {
    DYNAM.snapshot( &ss_tok );

    int crt_state = 0;

    for( auto& sw : dyn.switches )
        ( crt_state |= sw.dwn ) <<= 1;

    for( auto& js : dyn.joysticks ) { 
        ( crt_state |=  js.sw.dwn ) <<= 1;
        ( crt_state |=  abs( js.x ) >= barra::JS_HYST_HIGH ) <<= 1;
        ( crt_state |=  abs( js.y ) >= barra::JS_HYST_HIGH ) <<= 1;
    }

    if( crt_state != 0 && last_state != crt_state ) 
        BITNA.target( reacts[ ++react_at %= REACT_COUNT ] );
    else if( crt_state == 0 ) 
        BITNA.target( Led_BLK );

    last_state = crt_state;
} } },

FELLOW_TASK_led_controller{ "led_controller", 1024, Priority_Aesth, [] ( void* ) static -> void {
    const float Kp  = 0.36;
    const int   Ts  = 30;
    Led_        cmd = 0;
    float       err = 0;

for(;;) {
    err = BITNA._tar.r - BITNA._crt.r;
    cmd.r = BITNA._crt.r + err * Kp;

    err = BITNA._tar.g - BITNA._crt.g;
    cmd.g = BITNA._crt.g + err * Kp;

    err = BITNA._tar.b - BITNA._crt.b;
    cmd.b = BITNA._crt.b + err * Kp;

    BITNA.make( cmd );

    vTaskDelay( Ts );
} } },

FELLOW_TASK_wave_controller{ "wave_controller", 1024, Priority_AbvMain, [] ( void* ) static -> void {
    _MIRU::_wave_desc_t   desc;

for(;;) {
    if( xQueueReceive( MIRU._wave_q, &desc, portMAX_DELAY ) != pdPASS ) {
        vTaskDelay( 1000 ); continue;
    }

    MIRU.arm( const_cast< int* >( &desc.frags[ 0 ].freq ) );

    vTaskDelay( desc.frags[ 0 ].ms );

    for( int idx = 1; idx < desc.frag_count; ++idx ) {
        MIRU.make( desc.frags[ idx ].freq );
        vTaskDelay( desc.frags[ idx ].ms );
    }

    MIRU.disarm( NULL );
} } };


#define SPOT_LOOP_OVR  virtual void spot_loop( _SPOT::bli_t* bli ) override
#define SPOT_BEGIN_OVR virtual void spot_begin( void ) override
#define SPOT_END_OVR   virtual void spot_end( void ) override

struct _SPOT {
    struct bli_t {
        int   ms   = 0;

        struct _tim_t {
            int   _count   = 0;

            void each( int ms, std::function< void( void ) > func ) {
                if( _count < ms ) return;
                func();
                _count = 0;
            }
        }    tim[ 3 ]; 
        _tim_t& tim_0 = tim[ 0 ], tim_1 = tim[ 1 ], tim_2 = tim[ 2 ];
    };

    virtual void _spot_loop( bli_t* bli ) = 0; virtual void _spot_begin( void ) = 0; virtual void _spot_end( void ) = 0;
    virtual void spot_loop( bli_t* bli ) {}; virtual void spot_begin( void ) {}; virtual void spot_end( void ) {};

    inline static struct {
        TaskHandle_t   handle;
        _SPOT*         spot;
    } _sync{ handle: NULL, spot: NULL };

    static void _main( void* arg ) {
        _SPOT* crt     = NULL;
        bli_t   bli;
        int     last_ms = millis();

    l_swap:
        crt = _sync.spot;
        if( crt == NULL ) {
            ulTaskNotifyTake( true, portMAX_DELAY );
            goto l_swap;
        }

        crt->_spot_begin();
        for(; _sync.spot == crt ;) { 
            bli.ms = millis();
            for( auto& t : bli.tim ) t._count += bli.ms - last_ms;
            last_ms = bli.ms;

            crt->_spot_loop( &bli ); 
            vTaskDelay( CONFIG.spot_T );
        }
        crt->_spot_end();

        goto l_swap;
    };

    static void place( _SPOT* spot ) {
        if( _sync.spot != spot && ( _sync.spot = spot ) != NULL ) xTaskNotifyGive( _sync.handle );
    }
};


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


void loop() { 
    vTaskPrioritySet( NULL, Priority_Main );
    xSemaphoreGive( CONFIG.init_sem );

    xTaskCreate( &_SPOT::_main, "spot_main", 4096, NULL, Priority_SubMain, &_SPOT::_sync.handle );

    attachInterrupt( GPIO::xabara, [] ( void ) static -> void { 
        CONFIG_BRDG_MODE.bridge_back();
    },
        RISING 
    );

    for(;;) {
        Mode_ mode = ( Mode_ )CONFIG.mode.load( std::memory_order_seq_cst );

        if( mode != Mode_Brdg ) BITNA.x_set(); else BITNA.x_reset();

        CONFIG.mains[ ( int )mode ]( CONFIG._arg ); 

        _SPOT::place( NULL );
        YUNA.clear_w();
    }
    _dead(); 
}



/* .__   _  .  . .__
|| | ._ |_| |\/| |_
|| |__| | | |  | |__
*/ 
#define GAME_MAIN_OVR void main( void ) override

struct _GAME : _SPOT {
    virtual void main( void ) = 0;

    void _spot_begin( void ) override { 
        YUNA.clearDisplay();
        this->spot_begin(); 
    }

    void _spot_end( void ) override { 
        this->spot_end(); 
    }

    void _spot_loop( _SPOT::bli_t* bli ) override {
        this->spot_loop( bli );
        YUNA.display();
    }
};

#pragma region GAMES

struct _GAME_HEART : _GAME {
GAME_MAIN_OVR{
    _FELLOW_TASK::require( FellowTask_DynamScan | FellowTask_LedController | FellowTask_WaveController );
    BITNA.target( Led_BLK );

    const int    Ts     = 10;
    double       flt    = 0.0;
    double       damp   = 0.0;
    bool         riding = false;

MAIN_LOOP_ON( Mode_Game ) {
    int read = GPIO::A_R( GPIO::minju );

    read -= 1800;
    if( read < 0 ) read = 0;

    flt = flt * 0.95 + read * 0.05;
    damp = damp * 0.995 + read * 0.005;

    double diff = flt - damp;
    if( diff < 0 ) diff = 0;
    else diff = pow( diff, 2 );

    Serial.print( -100 );
    Serial.print( " " );
    Serial.print( 1500 );
    Serial.print( " " );
    Serial.println( read );

    if( diff >= 300.0 && !riding ) {
        static _MIRU::_wave_frag_t wave{ freq: 880, ms: 120 };

        riding = true;

        BITNA.make( Led_RED );
        MIRU.q_push( _MIRU::_wave_desc_t{ frags: &wave, frag_count: 1 } );
    } else if( diff <= 300.0 ) {
        riding = false;
    }
    
    vTaskDelay( Ts );
  }
};
} GAME_HEART;

struct _GAME_PONG : _GAME {
    inline static constexpr int     ARENA_Y_BEGIN   = 7;
    inline static constexpr float   ARENA_WIDTH     = 127.0;
    inline static constexpr float   ARENA_HEIGHT    = 56.0;

    inline static const _MIRU::_wave_frag_t   WAVE_PB_ARENA    = { freq: 130, ms: 80 };
    inline static const _MIRU::_wave_frag_t   WAVE_PB_RACHET   = { freq: 190, ms: 80 };
    inline static const _MIRU::_wave_frag_t   WAVE_START[ 3 ]  = {
        _MIRU::_wave_frag_t{ freq: 200, ms: 200 },
        _MIRU::_wave_frag_t{ freq: 250, ms: 200 },
        _MIRU::_wave_frag_t{ freq: 300, ms: 200 },
    };
    inline static const _MIRU::_wave_frag_t   WAVE_POINT[ 3 ]  = {
        _MIRU::_wave_frag_t{ freq: 300, ms: 200 },
        _MIRU::_wave_frag_t{ freq: 250, ms: 200 },
        _MIRU::_wave_frag_t{ freq: 200, ms: 200 },
    };

    struct _ball_t {
        float   x    = 0.0;
        float   y    = 0.0;
        float   vx   = 0.0;
        float   vy   = 0.0;

        inline float l( void ) const { return x - 2.5; }
        inline float r( void ) const { return x + 2.5; }
        inline float t( void ) const { return y - 2.5; }
        inline float b( void ) const { return y + 2.5; }
    } _PB;

    struct _rachet_t {
        float   y     = 0.0;
        float   ivy   = 0.0;

        void mov( float dy, float elapsed ) {
            float ny = y + dy * 42.0 * elapsed;

            if( ny - 11.0 < 0.0 ) ny = 11.0;
            if( ny + 11.0 > ARENA_HEIGHT ) ny = ARENA_HEIGHT - 11.0;

            ivy = dy * 12.0;
            y = ny;
        }

        inline float t( void ) const { return y - 11.0; }
        inline float b( void ) const { return y + 11.0; }
    } _RL, _RR;

    int _SL = 0, _SR = 0;

GAME_MAIN_OVR{
    _FELLOW_TASK::require( FellowTask_DynamScan | FellowTask_InputReact | FellowTask_LedController | FellowTask_WaveController );

    int                      last_us = 0;
    barra::dynamic_t         dyn;
    _DYNAM::snapshot_token_t ss_tok{ dst: &dyn, blk: true };

    auto rst_wait_2 = [ &, this ] ( int side ) -> void {
        _PB.x = ARENA_WIDTH / 2.0;
        _PB.y = ARENA_HEIGHT / 2.0;
        _PB.vx = 40.0 * side;
        _PB.vy = 0.0;

        MAIN_LOOP_ON( Mode_Game ) { 
            int now_ms = millis();
            _RL.y = ARENA_HEIGHT / 2.0 + sin( now_ms / 400.0 ) * 16.0;
            _RR.y = ARENA_HEIGHT / 2.0 + cos( now_ms / 460.0 ) * 16.0;

            this->splash(); 

            DYNAM.snapshot( &ss_tok ); 

            if( dyn.karina.rls ) _SL = _SR = 0;

            if( dyn.samantha.sw.dwn && dyn.rachel.sw.dwn ) {
                MIRU.q_push( _MIRU::_wave_desc_t{ frags: WAVE_START, frag_count: 3 } ); 
                break;
            }
        }

        _RL.y = _RR.y = ARENA_HEIGHT / 2.0;
    };

    rst_wait_2( 1 );

l_loop_begin:   
    last_us = micros();

MAIN_LOOP_ON( Mode_Game ) {
    int now_us = micros();
    float elapsed = ( float )( now_us - last_us ) / 1e6;
    last_us = now_us;

    DYNAM.snapshot( &ss_tok );

    if( abs( dyn.samantha.y ) > 0.1 ) _RR.mov( -dyn.samantha.y, elapsed );
    if( abs( dyn.rachel.y ) > 0.1 ) _RL.mov( -dyn.rachel.y, elapsed );

    switch( this->pb_mov( elapsed ) ) {
        case 0: break;

        case -1: ++_SR; rst_wait_2( 1 ); goto l_loop_begin;
        case 1: ++_SL; rst_wait_2( -1 ); goto l_loop_begin;
    }

    this->splash();
}

};

    void splash( void ) const {
        YUNA.clearDisplay();

        YUNA.drawFastVLine( 63, 0, 7, SSD1306_WHITE );
        YUNA.drawFastVLine( 64, 0, 7, SSD1306_WHITE );
        YUNA.drawFastHLine( 0, 6, YUNA.WIDTH, SSD1306_WHITE );

        for( int h = 9; h <= YUNA.HEIGHT; h += 5 ) {
            YUNA.drawPixel( 63, h, SSD1306_WHITE );
            YUNA.drawPixel( 64, h, SSD1306_WHITE );
        }

        YUNA.printf_av( YunaAV_TR, 58, 0, "%d", _SL );
        YUNA.printf_av( YunaAV_TL, 69, 0, "%d", _SR );

        YUNA.fillRect( ( int )_PB.l(), ARENA_Y_BEGIN + ( int )_PB.t(), 5, 5, SSD1306_WHITE );
        YUNA.fillRect( 0, ARENA_Y_BEGIN + ( int )_RL.t(), 2, 23, SSD1306_WHITE );
        YUNA.fillRect( 126, ARENA_Y_BEGIN + ( int )_RR.t(), 2, 23, SSD1306_WHITE );

        YUNA.display();
    }

    int pb_mov( float elapsed ) {
        _PB.x += _PB.vx * elapsed;
        _PB.y += _PB.vy * elapsed;

        auto y_collide = [ this ] ( const _rachet_t& r ) -> bool {
            return _PB.b() >= r.t() && _PB.t() <= r.b(); 
        };

        if( _PB.r() <= 0.0 ) {
            MIRU.q_push( _MIRU::_wave_desc_t{ frags: WAVE_POINT, frag_count: 3 } );
            return -1;
        }
        if( _PB.l() >= 127.0 ) {
            MIRU.q_push( _MIRU::_wave_desc_t{ frags: WAVE_POINT, frag_count: 3 } );
            return 1;
        }

        if( _PB.l() <= 2.0 && y_collide( _RL ) && _PB.vx < 0.0 ) {
            _PB.vx *= -1.0;
            _PB.vy += _RL.ivy;
            MIRU.q_push( _MIRU::_wave_desc_t{ frags: &WAVE_PB_RACHET, frag_count: 1 } );
        } else if( _PB.r() >= 125.0 && y_collide( _RR ) && _PB.vx > 0.0 ) {
            _PB.vx *= -1.0;
            _PB.vy += _RR.ivy;
            MIRU.q_push( _MIRU::_wave_desc_t{ frags: &WAVE_PB_RACHET, frag_count: 1 } );
        }

        if( _PB.t() <= 0.0 || _PB.b() >= ARENA_HEIGHT ) {
            _PB.vy *= -1.0; 
            MIRU.q_push( _MIRU::_wave_desc_t{ frags: &WAVE_PB_ARENA, frag_count: 1 } );
        }

        _PB.vx += 3.2 * elapsed * ( _PB.vx < 0.0 ? -1.0 : 1.0 );

        return 0;
    }

} GAME_PONG;

struct _GAME_SNAKE : _GAME {
GAME_MAIN_OVR{
    _FELLOW_TASK::require( FellowTask_DynamScan | FellowTask_InputReact | FellowTask_LedController | FellowTask_WaveController );

    barra::dynamic_t         dyn;
    _DYNAM::snapshot_token_t ss_tok{ dst: &dyn, blk: true };

MAIN_LOOP_ON( Mode_Game ) {
    
}

};
} GAME_SNAKE;

#pragma endregion GAMES

int _CONFIG_GAME_MODE::init( void ) {
	return 0;
}

void main_game( void* arg ) {
    ( ( _GAME* )arg )->main();
}



/* .__ __.__ ._  .
|| |     |   |_| |
|| |__   |   | \ |__
*/ 
#if BARRUNCUDA_WJP_VER == 1
static WJP_QGSTBL_ENTRY wjp_QGSTBL[ 4 ] = {
{ 
	str_id: "BITNA_CRT", 
	sz: 1, 
	read_only: WJP_QGSTBL_READ_ONLY, 
	qset_func: NULL,
	qget_func: NULL, 
	src: &BITNA._crt
},
{ 
	str_id: "GRAN_ACC_RANGE", 
	sz: 1, 
	read_only: WJP_QGSTBL_READ_WRITE, 
	qset_func: [] WJP_QSET_LAMBDA{ return GRAN.set_acc_range( ( MPU9250_accRange )*( uint8_t* )args.addr ) ? NULL : "GRAN_ACC_INVALID_RANGE"; },
	qget_func: NULL,
	src: NULL
},
{ 
	str_id: "GRAN_GYR_RANGE", 
	sz: 1, 
	read_only: WJP_QGSTBL_READ_WRITE, 
	qset_func: [] WJP_QSET_LAMBDA{ return GRAN.set_gyr_range( ( MPU9250_gyroRange )*( uint8_t* )args.addr ) ? NULL : "GRAN_GYR_INVALID_RANGE"; },
	qget_func: NULL,
	src: NULL
},
{
    str_id: "DYNAM",
    sz: sizeof( barra::dynamic_t ),
    read_only: WJP_QGSTBL_READ_ONLY,
    qset_func: NULL,
    qget_func: [] WJP_QGET_LAMBDA{
        static barra::dynamic_t         dyn;
        static _DYNAM::snapshot_token_t ss_tok{ dst: &dyn, blk: true };
        
        DYNAM.snapshot( &ss_tok );
        memcpy( dst, &dyn, sizeof( barra::dynamic_t ) );

        return NULL;
    },
    src: NULL
}
};

static WJP_IBRSTBL_ENTRY wjp_IBRSTBL[ 1 ] = {
{
    mem: NULL,
    sz: sizeof( WJP_HEAD ) + sizeof( barra::dynamic_t ),
    out_fnc: [] WJP_IBRST_OUT_LAMBDA { 
        static struct _PACKET : WJP_HEAD { 
            _PACKET() {
                WJP_HEAD::_dw0.op  = WJPOp_IBurst;
                WJP_HEAD::_dw1.seq = 0;
                WJP_HEAD::_dw3.sz  = sizeof( barra::dynamic_t ); 
            }
            barra::dynamic_t   dyn;
        } packet;
        static _DYNAM::snapshot_token_t ss_tok{ dst: &packet.dyn, blk: true };

        DYNAM.snapshot( &ss_tok );
        return &packet;
    },
    dir: WJP_IBRST_DIR_OUT,
    packed: WJP_IBRST_PACKED,
    _status: WJPIBrstStatus_Disengaged
}
};

int _CONFIG_CTRL_MODE::init( void ) {
	wjpblu.bind_qgstbl( WJP_QGSTBL{ 
      	entries: wjp_QGSTBL, 
      	count: sizeof( wjp_QGSTBL ) / sizeof( WJP_QGSTBL_ENTRY ) 
    } );

    wjpblu.bind_ibrstbl( WJP_IBRSTBL{
        entries: wjp_IBRSTBL,
        count: sizeof( wjp_IBRSTBL ) / sizeof( WJP_IBRSTBL_ENTRY )
    } );

    return wjpblu.init( 0 );
}

int wjp_loop( void ) {
    if( !CONFIG_CTRL_MODE.wjpblu.available() ) return 0;

    WJP_RESOLVE_RECV_INFO info;
    if( int ret = CONFIG_CTRL_MODE.wjpblu.resolve_recv( &info ); ret <= 0 ) {
        _printf( LogLevel_Error, "Protocol breach: %s.\n ", WJP_err_strs[ info.err ] );
        return ret;
    }

    if( info.nakr != NULL ) {
        _printf( LogLevel_Info, "Responded with NAK on seq (%d), op (%d). Reason: %s.\n ", ( int )info.recv_head._dw1.seq, ( int )info.recv_head._dw0.op, info.nakr );
        return 0;
    }

    switch( info.recv_head._dw0.op ) {
        case WJPOp_Heart: {
            _printf( LogLevel_Info, "Ping'd back on sequence (%d).\n", ( int )info.recv_head._dw1.seq );
        break; }
    }

    return 0;
}

std::function< int( void ) >   ctrl_loop_procs[]   = {
  /* 0b00 */ [] () static -> bool {
    CONFIG_CTRL_MODE._conn_rst = true;

    CONFIG_CTRL_MODE.wjpblu._ibrstbl[ 0 ]->_status = WJPIBrstStatus_Disengaged;
    
    BITNA.x_blink( 9, 50, 50, false, false );

    YUNA.clearDisplay();
    YUNA.drawBitmap( 0, 0, BARRA_BTH_SEARCH_STATIC, 91, 64, SSD1306_WHITE ); 
    YUNA.display();

    return true;
  },

  /* 0b01 */ [] () static -> bool {
    if( wjp_loop() != 0 ) {
      CONFIG_CTRL_MODE.wjpblu.disconnect();
      return true;
    }

    CONFIG_CTRL_MODE.wjpblu.ibrst( 0, 0 );

    return true;
  },

  /* 0b10 */ [] () static -> bool {
    static constexpr int SPLASH_OFS_AMP = 10;
    static int           splash_ofs     = 0;
    static bool          splash_dir     = 0;

    if( abs( splash_ofs += splash_dir ? 1 : -1 ) >= SPLASH_OFS_AMP ) splash_dir ^= 1; 

    YUNA.fillRect( 91, 0, 37, 64, SSD1306_BLACK );
    YUNA.drawBitmap( 103 + splash_ofs, 0, BARRA_BTH_SEARCH_DYNAMIC, 13, 64, SSD1306_WHITE );
    YUNA.display();

    BITNA.x_blink( 1, 100, 500, false, true );

    return true;
  },

  /* 0b11 */ [] () static -> bool {
    CONFIG_CTRL_MODE._conn_rst = false;
    
    BITNA.x_blink( 9, 50, 50, true, false );

    YUNA.drawBitmap( 0, 0, BARRA_BTH_CONNECTED, 128, 64, SSD1306_WHITE, SSD1306_BLACK ); 
    YUNA.display();

    return true;
  }
};

void main_ctrl( void* arg ) {
    _FELLOW_TASK::require( FellowTask_DynamScan | FellowTask_InputReact | FellowTask_LedController | FellowTask_WaveController );

    CONFIG_CTRL_MODE.wjpblu.begin( barra::DEVICE_NAME );    

    ctrl_loop_procs[ 0b00 ]();
MAIN_LOOP_ON( Mode_Ctrl ) {
    ctrl_loop_procs[ ( CONFIG_CTRL_MODE._conn_rst << 1 ) | CONFIG_CTRL_MODE.wjpblu.connected() ]();
} 
    ctrl_loop_procs[ 0b00 ]();

    CONFIG_CTRL_MODE.wjpblu.end();
}
#endif

#if BARRUNCUDA_WJP_VER == 3
int _CONFIG_CTRL_MODE::init( void ) {
    return 0x0;
}
void main_ctrl( void* arg ) {
    _FELLOW_TASK::require( FellowTask_DynamScan | FellowTask_InputReact | FellowTask_LedController | FellowTask_WaveController );

    rp::BLE_UART_Virtual_commander rp_vcmd;
    rp_vcmd.begin();
    rnk::status_t status = rp_vcmd.uplink( rp::TAG, 10000 ); 

    barra::dynamic_t         dyn;
    _DYNAM::snapshot_token_t ss_tok{ dst: &dyn, blk: true };

    struct {
        bool   flag   = false;
        bool   hold   = false;
    } headlights;
    struct {
        uint8_t   mode   = RP_TRACK_MODE_DECOUPLED;
    } track;
    struct {
        uint8_t   play : 1;
        uint8_t   prev : 1;
        uint8_t   next : 1;
    } wave;

MAIN_LOOP_ON( Mode_Ctrl ) {
    DYNAM.snapshot( &ss_tok );

    if( !dyn.rachel.sw.dwn ) {
        if( dyn.giselle.prs ) {
            headlights.flag ^= true;
        } else if( dyn.karina.dwn ) {
            headlights.flag = true;
            headlights.hold = true;
        } else if( headlights.hold ) {
            headlights.flag = false;
            headlights.hold = false;
        }
        wave.play = wave.prev = wave.next = 0x0;
    } else {
        wave.play = dyn.winter.dwn;
        wave.prev = dyn.giselle.dwn;
        wave.next = dyn.ningning.dwn;
    }

    if( dyn.samantha.sw.prs ) track.mode ^= 0x1;

    rp_vcmd.push( rp::virtual_commander_t{
        track_left:      dyn.rachel.y,
        track_right:     track.mode == RP_TRACK_MODE_DECOUPLED ? dyn.samantha.y : dyn.samantha.x,
        track_pwr:       dyn.tanya.lvl,
        track_mode:      track.mode,
        headlight_left:  headlights.flag ? 1.0 : 0.0,
        headlight_right: headlights.flag ? 1.0 : 0.0,
        wave_play:       wave.play,
        wave_prev:       wave.prev,
        wave_next:       wave.next
    } );
}
    rp_vcmd.downlink();
}
#endif


/* ._  ._  .   .__
|| |_\ |_| |\  | ._
|| |_| | \ |_\ |__|
*/ 
struct _bridge_dys_t {
    barra::joystick_t&   js;
    barra::switch_t&     sw_A;
    barra::switch_t&     sw_B;
    float&               level;
};

#define BRIDGE_BEGIN_OVR  virtual void begin() override
#define BRIDGE_END_OVR    virtual void end() override
#define BRIDGE_SELECT_OVR virtual void select() override
#define BRIDGE_DYS_OVR    virtual void give_dys( _bridge_dys_t* dys ) override
struct _BRIDGE : _SPOT {
    _BRIDGE*      sup                                 = NULL;
    _BRIDGE*      subs[ _CONFIG_BRDG_MODE::STRIDE ]   = { ( memset( ( void* )subs, NULL, _CONFIG_BRDG_MODE::STRIDE * sizeof( void* ) ), ( _BRIDGE* )NULL ) };
    int           sub_idx                             = 0;
    const char*   name                                = "root";
    int           _last_ms                            = 0;

    static constexpr int   _NAV_V_X   = 1;
    static constexpr int   _NAV_V_Y   = 1;
    static constexpr int   _NAV_V_W   = 7;
    static constexpr int   _NAV_V_H   = 56;

    static constexpr int   _NAV_H_X   = 120;
    static constexpr int   _NAV_H_Y   = 1;
    static constexpr int   _NAV_H_W   = 7;
    static constexpr int   _NAV_H_H   = 56;

    static constexpr int   _NAV_P_X   = 1;
    static constexpr int   _NAV_P_Y   = 59;

    static constexpr int   _CAGE_X    = 11;
    static constexpr int   _CAGE_Y    = 2;
    static constexpr int   _CAGE_MX   = 64;
    static constexpr int   _CAGE_MY   = 27;

    static constexpr int   _ICO_X     = 15;
    static constexpr int   _ICO_Y     = 6;
    
    void _spot_begin( void ) override { 
        YUNA.clearDisplay();

        if( this != CONFIG_BRDG_MODE._home ) {
            int x = _NAV_H_X, y = _NAV_H_Y;
            for( int count = 0; count < _CONFIG_BRDG_MODE::STRIDE && this->sup->subs[ count ] != NULL ; ++count ) {
                if( this == this->sup->subs[ count ] ) {
                    YUNA.drawRect( x, y, 7, 7, SSD1306_WHITE );
                    y += 9;
                } else {
                    YUNA.drawRect( x + 2, y, 3, 3, SSD1306_WHITE );
                    y += 5;
                }
            }

            x = _NAV_V_X; y = _NAV_V_Y;
            for( int count = 0; count <= CONFIG_BRDG_MODE._nav.dep; ++count ) {
                if( count == CONFIG_BRDG_MODE._nav.dep ) {
                    YUNA.drawRect( x, y, 7, 7, SSD1306_WHITE );
                    y += 9;
                } else {
                    YUNA.drawRect( x + 2, y, 3, 3, SSD1306_WHITE );
                    y += 5;
                }
            }
            if( this->subs[ 0 ] != NULL ) 
                YUNA.drawTriangle( x + 1, y, x + 5, y, x + 3, y + 6, SSD1306_WHITE );
            else
                YUNA.drawRect( x + 1, y, 5, 2, SSD1306_WHITE );

            YUNA.setCursor( _NAV_P_X, _NAV_P_Y );
            YUNA.print( CONFIG_BRDG_MODE._nav.path );
        } 

        this->spot_begin(); 
    }

    void _spot_end( void ) { 
        this->spot_end(); 
    }

    void _spot_loop( _SPOT::bli_t* bli ) {
        if( this != CONFIG_BRDG_MODE._home ) {
            YUNA.fillRect( _CAGE_X - 2, _CAGE_Y - 2, BARRA_BRDG_CAGE_W + 4, BARRA_BRDG_CAGE_H + 4, SSD1306_BLACK );

            float cage_arg = ( float )bli->ms / 100;
            int cage_ox = sin( cage_arg ) * 2;
            int cage_oy = cos( cage_arg ) * 2;
            YUNA.drawBitmap( _CAGE_X + cage_ox, _CAGE_Y + cage_oy, BARRA_BRDG_CAGE, BARRA_BRDG_CAGE_W, BARRA_BRDG_CAGE_H, SSD1306_WHITE );
        } 

        this->spot_loop( bli );
        YUNA.display();
    }

    virtual void begin( void ) {}
    virtual void end( void ) {}

    virtual void select( void ) {}

    virtual void give_dys( _bridge_dys_t* dys ) {}

} _BRIDGE_ROOT;

#pragma region BRIDGES

struct _BRIDGE_HOME : _BRIDGE{ 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( 0, 0, BARRA_LOGO_VIBE, 20, 64, SSD1306_WHITE );
    YUNA.drawBitmap( 108, 0, BARRA_LOGO_VIBE, 20, 64, SSD1306_WHITE );

    float vibe_amp = ( 1.1 + sin( bli->ms / 1200 ) ) * 8.0 + 8.0;
    int vibe_lo = ( 1.1 + sin( bli->ms / 230.0 ) ) * vibe_amp;
    int vibe_ro = ( 1.1 + cos( bli->ms / 230.0 ) ) * vibe_amp;

    YUNA.fillRect( 0, 0, 20, vibe_lo, SSD1306_BLACK );
    YUNA.fillRect( 108, 0, 20, vibe_ro, SSD1306_BLACK );
};
SPOT_BEGIN_OVR{
    YUNA.drawBitmap( 0, 0, BARRA_LOGO, 128, 64, SSD1306_WHITE );
    YUNA.fillRect( 0, 0, 20, 64, SSD1306_BLACK );
    YUNA.fillRect( 108, 0, 20, 64, SSD1306_BLACK );
};
} BRIDGE_HOME;

struct _BRIDGE_BTH : _BRIDGE { 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_BTH, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
};
BRIDGE_SELECT_OVR{
    CONFIG.xchg_mode( Mode_Ctrl, NULL );
};
} BRIDGE_BTH;

struct _BRIDGE_FORGERY : _BRIDGE { 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_FORGERY, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
};
BRIDGE_SELECT_OVR{
    
};
} BRIDGE_FORGERY;

struct _BRIDGE_TOOLS : _BRIDGE { 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_TOOLS, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
}; 
} BRIDGE_TOOLS;

struct _BRIDGE_GAMES : _BRIDGE { 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_GAMES, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
};
} BRIDGE_GAMES;

struct _BRIDGE_ENV : _BRIDGE {
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_ENV, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
};
} BRIDGE_ENV;

struct _BRIDGE_CHILL : _BRIDGE { 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_SYSINFO, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
}; 
} BRIDGE_CHILL;

struct _BRIDGE_LASER : _BRIDGE {
BRIDGE_BEGIN_OVR{ 
    SUZYQ.disarm( NULL ); 
};
BRIDGE_END_OVR{ 
    SUZYQ.disarm( NULL );
};
SPOT_LOOP_OVR{
    YUNA.drawBitmap( _ICO_X + 2, _ICO_Y + 6, BARRA_BRDG_ICO_LASER, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );

    YUNA.drawBitmap( 
        _CAGE_MX - BARRA_MISC_DIS_ARMED_W/2, _CAGE_MY - BARRA_MISC_DIS_ARMED_H/2 - 16,
        SUZYQ.is_armed( std::memory_order_relaxed ) ? BARRA_MISC_ARMED : BARRA_MISC_DISARMED,
        BARRA_MISC_DIS_ARMED_W, BARRA_MISC_DIS_ARMED_H, 
        SSD1306_WHITE 
    );
};
BRIDGE_SELECT_OVR{
    SUZYQ.is_armed() ? SUZYQ.disarm( NULL ) : SUZYQ.arm( NULL );
};
BRIDGE_DYS_OVR{
    if( dys->sw_B.prs ) SUZYQ.arm( NULL );
    else if( dys->sw_B.rls ) SUZYQ.disarm( NULL );
};
} BRIDGE_LASER;

struct _BRIDGE_SONAR : _BRIDGE { 
    int   mms   = 0.0;

SPOT_LOOP_OVR{ 
    bli->tim_0.each( 100, [ this ] ( void ) -> void {
        // int read = GOLANI.read();
        // mms = read != 0 ? read : mms;
    } );

    YUNA.setTextSize( 2 );
    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%d mm", mms );
    YUNA.setTextSize( 1 );
};
} BRIDGE_SONAR;

struct _BRIDGE_COMPASS : _BRIDGE { 
    float   degrees   = 0.0;

SPOT_LOOP_OVR{ 
    bli->tim_0.each( 100, [ this ] ( void ) -> void {
        degrees = CHIM.degs();
    } );

    YUNA.setTextSize( 2 );
    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f '", degrees );
    YUNA.setTextSize( 1 );
};
} BRIDGE_COMPASS;

struct _BRIDGE_DRAG : _BRIDGE { 
    float angleX, biasX;
    float angleY, biasY;

    float Pxx = 1, Pxb = 0, Pbb = 1;  // covariance matrix for X axis
    float Pyx = 1, Pyb = 0, Pby = 1;  // covariance matrix for Y axis

    float Q_angle = 0.001;  // process noise
    float Q_bias  = 0.003;
    float R_angle = 0.03;   // measurement noise

    unsigned long lastTime;

    float velX=0, velY=0, velZ=0;
float posX=0, posY=0, posZ=0;

    float accX_prev=0, accY_prev=0, accZ_prev=0;
float accXF=0, accYF=0, accZF=0;
float alpha = 0.98;
    void kalmanUpdate(float accAngle, float gyroRate, float &angle, float &bias,
                  float &Pxx, float &Pxb, float &Pbb, float dt) {

        // ---- Predict ----
        angle += (gyroRate - bias) * dt;

        Pxx += dt * (dt*Pbb - Pxb - Pxb + Q_angle);
        Pxb += dt * (-Pbb);
        Pbb += Q_bias * dt;

        // ---- Update ----
        float S = Pxx + R_angle;
        float Kx = Pxx / S;
        float Kb = Pxb / S;

        float y = accAngle - angle;

        angle += Kx * y;
        bias += Kb * y;

        float Pxx_new = Pxx - Kx * Pxx;
        float Pxb_new = Pxb - Kx * Pxb;

        Pxx = Pxx_new;
        Pxb = Pxb_new;
    }

SPOT_LOOP_OVR{ 
    auto dyn = DYNAM.snapshot_once();

    unsigned long now = micros();
    float dt = (now - lastTime) / 1000000.0;
    lastTime = now;

    // Convert raw data
    float accX = dyn.gran.acc.x;
    float accY = dyn.gran.acc.y;
    float accZ = dyn.gran.acc.z;

    float gyroX = dyn.gran.gyr.x;
    float gyroY = dyn.gran.gyr.y;

    // Calculate accelerometer angles
    float accAngleX = atan2(accY, accZ) * 180 / PI;
    float accAngleY = atan2(accX, accZ) * 180 / PI;

    // Kalman update
    kalmanUpdate(accAngleX, gyroX, angleX, biasX, Pxx, Pxb, Pbb, dt);
    kalmanUpdate(accAngleY, -gyroY, angleY, biasY, Pyx, Pyb, Pby, dt);

    // ---- Remove gravity to get linear acceleration ----
    float gX = sin(angleX * PI / 180.0) * 9.81;
    float gY = -sin(angleY * PI / 180.0) * 9.81;
    float gZ = cos(angleX * PI / 180.0) * cos(angleY * PI / 180.0) * 9.81;

    float linAccX = accX * 9.81 - gX;
    float linAccY = accY * 9.81 - gY;
    float linAccZ = accZ * 9.81 - gZ;

    bool isStill = 
    fabs(linAccX) < 0.08 &&
    fabs(linAccY) < 0.08 &&
    fabs(linAccZ) < 0.10;

    accXF = alpha * (accXF + linAccX - accX_prev);
    accYF = alpha * (accYF + linAccY - accY_prev);
    accZF = alpha * (accZF + linAccZ - accZ_prev);

    accX_prev = linAccX;
    accY_prev = linAccY;
    accZ_prev = linAccZ;

    // ---- Integrate to velocity ----
    velX += accXF * dt;
    velY += accYF * dt;
    velZ += accZF * dt;

    posX += velX * dt;
    posY += velY * dt;
    posZ += velZ * dt;

    if (isStill) {
        velX = 0;
        velY = 0;
        velZ = 0;
    }

    if (isStill) {
    posX = posX;
    posY = posY;
    posZ = posZ;
}


    YUNA.setTextSize( 1 );
    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f", posX );
    YUNA.setTextSize( 1 );
};
} BRIDGE_DRAG;

struct _BRIDGE_WAVE : _BRIDGE {
    inline static constexpr int   FREQ_LOW[ 2 ]    = { 1, 20 };
    inline static constexpr int   FREQ_HIGH[ 2 ]   = { 20, 4'200 };

    int    freq    = 0;
    bool   mode    = 0; 

BRIDGE_BEGIN_OVR{ 
    FELLOW_TASK_wave_controller.suspend();
    MIRU.disarm( NULL ); 
};
BRIDGE_END_OVR{ 
    detachInterrupt( GPIO::miru );
    MIRU.disarm( NULL ); 
    FELLOW_TASK_wave_controller.resume();
};
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( 
        _CAGE_MX - BARRA_MISC_DIS_ARMED_W/2, _CAGE_MY - BARRA_MISC_DIS_ARMED_H/2 - 14,
        MIRU.is_armed( std::memory_order_relaxed ) ? BARRA_MISC_ARMED : BARRA_MISC_DISARMED,
        BARRA_MISC_DIS_ARMED_W, BARRA_MISC_DIS_ARMED_H, 
        SSD1306_WHITE 
    );

    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%s", mode == 0 ? "prercision >" : "< range" );
    
    YUNA.setTextSize( 2 );
    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY + 14, "%d Hz", freq );
    YUNA.setTextSize( 1 );
};
BRIDGE_SELECT_OVR{
    if( MIRU.is_armed() == false ) {
        MIRU.arm( &freq ); 
        attachInterrupt( GPIO::miru, [] ( void ) static -> void { BITNA.make( Led_ORANGE ); }, FALLING );
    } else { 
        detachInterrupt( GPIO::miru );
        MIRU.disarm( NULL );
    }
};
BRIDGE_DYS_OVR{
    static float last_tuner = -1000.0;

    if( dys->js.edg.is == 1 ) {
        if( dys->js.edg.x == 1 && mode == 0 ) { mode = 1; last_tuner = -1000.0; }
        else if( dys->js.edg.x == -1 && mode == 1 ) { mode = 0; last_tuner = -1000.0; } 
    }

    switch( mode ) {
        case 0: {
            freq = dys->level * ( FREQ_HIGH[ 0 ] - FREQ_LOW[ 0 ] ) + FREQ_LOW[ 0 ];
            if( freq != last_tuner ) { last_tuner = freq; goto l_make; }
        break; }

        case 1: {
            if( abs( dys->level - last_tuner ) < 0.02 ) break;

            last_tuner = dys->level;
            freq = pow( dys->level, 2 ) * ( FREQ_HIGH[ 1 ] - FREQ_LOW[ 1 ] ) + FREQ_LOW[ 1 ];
        [[fallthrough]]; }

        l_make: {
            if( MIRU.is_armed( std::memory_order_relaxed ) ) MIRU.make( freq );
        }
    }
};
} BRIDGE_WAVE;

struct _BRIDGE_HEART : _BRIDGE {
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_HEART, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
};
BRIDGE_SELECT_OVR{
    CONFIG.xchg_mode( Mode_Game, &GAME_HEART );
};
} BRIDGE_HEART;

struct _BRIDGE_TEMPERATURE : _BRIDGE { 
    float   read         = 0.0;
    bool    in_celsius   = true;

SPOT_LOOP_OVR{ 
    bli->tim_0.each( 300, [ this ] ( void ) -> void {
        read = SUSAN.readTemperature();
    } );

    YUNA.setTextSize( 2 );
    if( in_celsius ) {
        YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f 'c", read );
    } else {
        YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f 'f", read * 1.8 + 32.0 );
    }
    YUNA.setTextSize( 1 );
};
BRIDGE_SELECT_OVR{
    in_celsius ^= true;
};
} BRIDGE_TEMPERATURE;

struct _BRIDGE_PRESSURE : _BRIDGE { 
    float   read   = 0.0;
    int     unit   = 0;

SPOT_LOOP_OVR{ 
    bli->tim_0.each( 300, [ this ] ( void ) -> void {
        read = SUSAN.readPressure();
    } );

    YUNA.setTextSize( 2 );
    switch( unit ) {
        case 0: YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.1f hPa", read * 1e-2 ); break;
        case 1: YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f bar", read * 1e-5 ); break;
        case 2: YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f atm", read * 9.86923267e-6 ); break;
    }
    YUNA.setTextSize( 1 );
};
BRIDGE_SELECT_OVR{
    if( ++unit >= 3 ) unit = 0;
};
} BRIDGE_PRESSURE;

struct _BRIDGE_HUMIDITY : _BRIDGE { 
    float   read   = 0.0;
SPOT_LOOP_OVR{ 
    bli->tim_0.each( 300, [ this ] ( void ) -> void {
        read = SUSAN.readHumidity();
    } );

    YUNA.setTextSize( 2 );
    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f %%", read );
    YUNA.setTextSize( 1 );
};
} BRIDGE_HUMIDITY;

struct _BRIDGE_ALTITUDE : _BRIDGE { 
    float   read   = 0.0;

SPOT_LOOP_OVR{ 
    bli->tim_0.each( 300, [ this ] ( void ) -> void {
        read = SUSAN.readAltitude( 1013.25 );
    } );

    YUNA.setTextSize( 2 );
    YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, "%.2f m", read );
    YUNA.setTextSize( 1 );
};
} BRIDGE_ALTITUDE;

struct _BRIDGE_LIGHT : _BRIDGE { 
    float   read   = 0.0;

SPOT_LOOP_OVR{ 
    bli->tim_0.each( 300, [ this ] ( void ) -> void {
        read = DYNAM.scan( Scan_Light ).naksu.lvl;
    } );

    YUNA.setTextSize( 2 );
    
    const char* fmt = NULL;
    switch( _NAKSU::_unit ) {
        case _NAKSU::Unit_Raw: fmt = "%.0f raw"; YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, fmt, read );         break;
        case _NAKSU::Unit_ELA: fmt = "%.2f%%";   YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, fmt, read * 100.0 ); break;
        case _NAKSU::Unit_Lux: fmt = "%.2f lux"; YUNA.printf_av( YunaAV_C, _CAGE_MX, _CAGE_MY, fmt, read );         break;
    }

    YUNA.setTextSize( 1 );
};
BRIDGE_SELECT_OVR{
    _NAKSU::Unit_ nu = ( _NAKSU::Unit_ )( _NAKSU::_unit + 1 );
    if( nu >= _NAKSU::UNIT_COUNT ) nu = _NAKSU::Unit_Raw;
    _NAKSU::set_unit( nu );
};
} BRIDGE_LIGHT;

struct _BRIDGE_PONG : _BRIDGE { 
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_PONG, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
}; 
BRIDGE_SELECT_OVR{
    CONFIG.xchg_mode( Mode_Game, &GAME_PONG );
};
} BRIDGE_PONG;

struct _BRIDGE_SNAKE : _BRIDGE {
SPOT_LOOP_OVR{ 
    YUNA.drawBitmap( _ICO_X, _ICO_Y, BARRA_BRDG_ICO_SNAKE, BARRA_BRDG_ICO_W, BARRA_BRDG_ICO_H, SSD1306_WHITE );
}; 
BRIDGE_SELECT_OVR{
    CONFIG.xchg_mode( Mode_Game, &GAME_SNAKE );
};
} BRIDGE_SNAKE;

struct _BRIDGE_MP3 : _BRIDGE {
    inline static const _MIRU::_wave_frag_t   WAVE_AT_DOOMS_GATE[ 28 ]  = {
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 164, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 146, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 130, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 116, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 123, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 130, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 164, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 146, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 130, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 82, ms: 272 },
        _MIRU::_wave_frag_t{ freq: 68, ms: 672 }
    };

SPOT_LOOP_OVR{ 
    
};
BRIDGE_SELECT_OVR{
    MIRU.q_push( _MIRU::_wave_desc_t{ frags: WAVE_AT_DOOMS_GATE, frag_count: 28 } );
};
BRIDGE_DYS_OVR{
    
};
} BRIDGE_MP3;

#pragma endregion BRIDGES


int _CONFIG_BRDG_MODE::init( void ) {
    _root = &_BRIDGE_ROOT;
    _nav.crt = _home = &BRIDGE_HOME;

    _BRIDGE_ROOT.sub_idx = 1;
    _BRIDGE_ROOT.subs[ 0 ] = &BRIDGE_BTH;
    _BRIDGE_ROOT.subs[ 1 ] = &BRIDGE_HOME;
    _BRIDGE_ROOT.subs[ 2 ] = &BRIDGE_FORGERY;

    BRIDGE_BTH.name = "bluetooth";

    BRIDGE_FORGERY.name = "forgery";

    BRIDGE_HOME.name = "";
    BRIDGE_HOME.sup = BRIDGE_BTH.sup = BRIDGE_FORGERY.sup = &_BRIDGE_ROOT;
    BRIDGE_HOME.subs[ 0 ] = &BRIDGE_TOOLS;
    BRIDGE_HOME.subs[ 1 ] = &BRIDGE_GAMES;
    BRIDGE_HOME.subs[ 2 ] = &BRIDGE_ENV;
    BRIDGE_HOME.subs[ 3 ] = &BRIDGE_CHILL;

    BRIDGE_TOOLS.name = "tools";
    BRIDGE_TOOLS.sup = &BRIDGE_HOME;
    BRIDGE_TOOLS.subs[ 0 ] = &BRIDGE_LASER;
    BRIDGE_TOOLS.subs[ 1 ] = &BRIDGE_SONAR;
    BRIDGE_TOOLS.subs[ 2 ] = &BRIDGE_COMPASS;
    BRIDGE_TOOLS.subs[ 3 ] = &BRIDGE_DRAG;
    BRIDGE_TOOLS.subs[ 4 ] = &BRIDGE_WAVE;
    BRIDGE_TOOLS.subs[ 5 ] = &BRIDGE_HEART;

    BRIDGE_GAMES.name = "games";
    BRIDGE_GAMES.sup = &BRIDGE_HOME;
    BRIDGE_GAMES.subs[ 0 ] = &BRIDGE_PONG;
    BRIDGE_GAMES.subs[ 1 ] = &BRIDGE_SNAKE;

    BRIDGE_ENV.name = "env";
    BRIDGE_ENV.sup = &BRIDGE_HOME;
    BRIDGE_ENV.subs[ 0 ] = &BRIDGE_TEMPERATURE;
    BRIDGE_ENV.subs[ 1 ] = &BRIDGE_PRESSURE;
    BRIDGE_ENV.subs[ 2 ] = &BRIDGE_HUMIDITY;
    BRIDGE_ENV.subs[ 3 ] = &BRIDGE_ALTITUDE;
    BRIDGE_ENV.subs[ 4 ] = &BRIDGE_LIGHT;

    BRIDGE_CHILL.name = "chill";
    BRIDGE_CHILL.sup = &BRIDGE_HOME;
    BRIDGE_CHILL.subs[ 0 ] = &BRIDGE_MP3;

    BRIDGE_LASER.name = "laser";
    BRIDGE_LASER.sup = &BRIDGE_TOOLS;

    BRIDGE_SONAR.name = "sonar";
    BRIDGE_SONAR.sup = &BRIDGE_TOOLS;

    BRIDGE_COMPASS.name = "compass";
    BRIDGE_COMPASS.sup = &BRIDGE_TOOLS;

    BRIDGE_DRAG.name = "drag";
    BRIDGE_DRAG.sup = &BRIDGE_TOOLS;

    BRIDGE_WAVE.name = "wave";
    BRIDGE_WAVE.sup = &BRIDGE_TOOLS;

    BRIDGE_HEART.name = "heart";
    BRIDGE_HEART.sup = &BRIDGE_TOOLS;

    BRIDGE_TEMPERATURE.name = "temperature";
    BRIDGE_TEMPERATURE.sup = &BRIDGE_ENV;

    BRIDGE_PRESSURE.name = "pressure";
    BRIDGE_PRESSURE.sup = &BRIDGE_ENV;

    BRIDGE_HUMIDITY.name = "humidity";
    BRIDGE_HUMIDITY.sup = &BRIDGE_ENV;

    BRIDGE_ALTITUDE.name = "altitude";
    BRIDGE_ALTITUDE.sup = &BRIDGE_ENV;

    BRIDGE_LIGHT.name = "light";
    BRIDGE_LIGHT.sup = &BRIDGE_ENV;

    BRIDGE_PONG.name = "pong";
    BRIDGE_PONG.sup = &BRIDGE_GAMES;

    BRIDGE_SNAKE.name = "snake";
    BRIDGE_SNAKE.sup = &BRIDGE_GAMES;

    BRIDGE_MP3.name = "mp3";
    BRIDGE_MP3.sup = &BRIDGE_CHILL;

    return 0;
}

void _CONFIG_BRDG_MODE::bridge_back() { 
    Mode_ last_mode = CONFIG.xchg_mode( Mode_Brdg, NULL );
    // if( last_mode == Mode_Brdg ) {
    //     _root->sub_idx = 1;
    //     strcpy( _nav.path, ">//" );
    //     _nav.dep = 0;
    //     _nav.crt = _home;
    //     _SPOT::place( _home );
    // } 
}

int _CONFIG_BRDG_MODE::bridge_N() {
    if( _nav.crt->sup == NULL || _nav.crt->sup == &_BRIDGE_ROOT ) return -1;
    --_nav.dep;
    _nav.crt = _nav.crt->sup;
    _nav.path_s();
    return 0;
}
int _CONFIG_BRDG_MODE::bridge_S() {
    if( _nav.crt->subs[ _nav.crt->sub_idx ] == NULL ) return -1;
    ++_nav.dep;
    _nav.crt = _nav.crt->subs[ _nav.crt->sub_idx ];
    _nav.path_d( _nav.crt->name, _nav.crt->subs[ 0 ] != NULL );
    return 0;
}
int _CONFIG_BRDG_MODE::bridge_E() {
    int next_idx = _nav.crt->sup->sub_idx + 1;
    if( next_idx >= STRIDE || _nav.crt->sup->subs[ next_idx ] == NULL ) return -1;
    _nav.crt->sup->sub_idx = next_idx;
    _nav.crt = _nav.crt->sup->subs[ next_idx ];
    _nav.path_x( _nav.crt->name, _nav.crt->subs[ 0 ] != NULL );
    return 0;
}
int _CONFIG_BRDG_MODE::bridge_W() {
    int next_idx = _nav.crt->sup->sub_idx - 1;
    if( next_idx < 0 || _nav.crt->sup->subs[ next_idx ] == NULL ) return -1;
    _nav.crt->sup->sub_idx = next_idx;
    _nav.crt = _nav.crt->sup->subs[ next_idx ];
    _nav.path_x( _nav.crt->name, _nav.crt->subs[ 0 ] != NULL );
    return 0;
}


void main_brdg( void* arg ) {
    _FELLOW_TASK::require( FellowTask_DynamScan | FellowTask_InputReact | FellowTask_LedController | FellowTask_WaveController );

    barra::dynamic_t         dyn            = {};
    _DYNAM::snapshot_token_t ss_tok         = { dst: &dyn, blk: true };
    _bridge_dys_t            dys            = { js: dyn.rachel, sw_A: dyn.giselle, sw_B: dyn.ningning, level: dyn.tanya.lvl };
    _BRIDGE*                 prev_nav_crt   = &BRIDGE_HOME;
    _MIRU::_wave_frag_t      nav_wave       = { freq: 160, ms: 80 };
    _MIRU::_wave_frag_t      sel_wave       = { freq: 200, ms: 80 };

    _SPOT::place( CONFIG_BRDG_MODE._nav.crt );

MAIN_LOOP_ON( Mode_Brdg ) {
    DYNAM.snapshot( &ss_tok );

    if( dyn.samantha.edg.is != 1 ) goto l_action;

    prev_nav_crt = CONFIG_BRDG_MODE._nav.crt;
    if     ( dyn.samantha.edg.x == 1 )  { if( CONFIG_BRDG_MODE.bridge_E() == 0 ) goto l_nav; }
    else if( dyn.samantha.edg.x == -1 ) { if( CONFIG_BRDG_MODE.bridge_W() == 0 ) goto l_nav; }
    else if( dyn.samantha.edg.y == 1 )  { if( CONFIG_BRDG_MODE.bridge_N() == 0 ) goto l_nav; }
    else if( dyn.samantha.edg.y == -1 ) { if( CONFIG_BRDG_MODE.bridge_S() == 0 ) goto l_nav; }
    goto l_action;
    
l_nav:
    prev_nav_crt->end();
    MIRU.q_push( _MIRU::_wave_desc_t{ frags: &nav_wave, frag_count: 1 } );
    CONFIG_BRDG_MODE._nav.crt->begin();
    _SPOT::place( CONFIG_BRDG_MODE._nav.crt );
    goto l_end;

l_action:
    if( dyn.samantha.sw.rls || ( CONFIG_BRDG_MODE._nav.crt->subs[ 0 ] == NULL && dyn.samantha.edg.y == -1 ) ) {
        MIRU.q_push( _MIRU::_wave_desc_t{ frags: &sel_wave, frag_count: 1 } );
        CONFIG_BRDG_MODE._nav.crt->select();
        goto l_end;
    }

    CONFIG_BRDG_MODE._nav.crt->give_dys( &dys );

l_end:
    continue;
}

}
