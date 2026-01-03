#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#define DWMQ_NAMESPACE namespace dwmq


DWMQ_NAMESPACE {


inline const char* const   TAG   = "[DWMQ]";

typedef   int       status_t;
typedef   int32_t   ir_signal_t;

enum TaskPriority_ {
    TaskPriority_Idle = tskIDLE_PRIORITY,

    TaskPriority_WhenFeelingLikeIt,
    TaskPriority_Effect,
    TaskPriority_Current,
    TaskPriority_Urgent
};


};


#define DWMQ_inline inline

#define DWMQ_ASSERT_OR( c ) if( !(c) )

#define DWMQ_YIELD_CORE taskYIELD()


#include <string>
#include <tuple>
#include <atomic>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include <driver/uart.h>

#include <EEPROM.h>
#include <BluetoothSerial.h>
#include <Wire.h>


DWMQ_NAMESPACE {


class MIRUN {
public:
    struct ECHO {
        void init( void ) {
            esp_log_level_set( "*", ESP_LOG_INFO ); 
            Serial.begin( 115200 );
        }
        
        template< typename ...Args > DWMQ_inline void err( const char* fmt, Args&&... args ) {
            Serial.printf( "%s [ error ] -> ", TAG );
            Serial.printf( fmt, std::forward< Args >( args )... );
            Serial.println();
        }

        template< typename ...Args > DWMQ_inline void wrn( const char* fmt, Args&&... args ) {
            Serial.printf( "%s [ warning ] -> ", TAG );
            Serial.printf( fmt, std::forward< Args >( args )... );
            Serial.println();
        }

        template< typename ...Args > DWMQ_inline void inf( const char* fmt, Args&&... args ) {
            Serial.printf( "%s [ info ] -> ", TAG );
            Serial.printf( fmt, std::forward< Args >( args )... );
            Serial.println();
        }

    } Echo;

public:
    struct CONFIG {
        struct IR_REMOTE {
            static constexpr ir_signal_t   SIG_SELECT_DEVICE    = 0x0;
            static constexpr ir_signal_t   SIG_IMPULSE_POWER    = 0x15;
            static constexpr ir_signal_t   SIG_BLUE_ENGAGE      = 0x26;
            static constexpr ir_signal_t   SIG_BLUE_DISENGAGE   = 0x27;
            static constexpr ir_signal_t   SIG_BLUE_TOGGLE      = 0x24;
            static constexpr ir_signal_t   SIG_FANS_ENGAGE      = -0x2;
            static constexpr ir_signal_t   SIG_FANS_DISENGAGE   = -0x3;
            static constexpr ir_signal_t   SIG_FANS_TOGGLE      = 0x25;
            static constexpr ir_signal_t   SIG_VOL_UP           = 0x12;
            static constexpr ir_signal_t   SIG_VOL_DOWN         = 0x13;

            static constexpr int           BEFORE_ENTRY_US      = 1000000;

            static constexpr int           STACK_MAIN           = 4096;
    
        } IrRemote;

        struct RELAY_GRID {
            static constexpr int   POWER_HOLD_MS   = 500; 

        } RelayGrid;

        struct HEAT_SINK {
            static constexpr int   STACK_MAIN   = 4096;
        } HeatSink;

        struct HYPERVISOR {
            static constexpr int   VOL_KEEP_ALIVE_US   = 300000;
        } HV;

        TwoWire*   I2C_bus   = nullptr;

    } Config;

public:
    status_t init( void ) {
        Echo.init();
        Echo.inf( "Preparing to dance..." );

        esp_task_wdt_deinit();

        Config.I2C_bus = &Wire;
        Config.I2C_bus->begin();

        Echo.inf( "Ready to dance." );
        return 0;
    }

} Mirun;


};
