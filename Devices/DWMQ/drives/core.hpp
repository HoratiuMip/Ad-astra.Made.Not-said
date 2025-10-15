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

typedef   int        status_t;
typedef   uint32_t   ir_signal_t;

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

#include <esp_task_wdt.h>
#include <driver/gpio.h>

#include <EEPROM.h>
#include <BluetoothSerial.h>


DWMQ_NAMESPACE {


class MIRUNA {
public:
    struct ECHO {
        void init( void ) {
            Serial.begin( 115200 );
            esp_log_level_set( "*", ESP_LOG_INFO ); 
        }
        
        template< typename ...Args > DWMQ_inline void err( const char* fmt, Args&&... args ) {
            ESP_LOGE( TAG, fmt, std::forward< Args >( args )... );
        }

        template< typename ...Args > DWMQ_inline void wrn( const char* fmt, Args&&... args ) {
            ESP_LOGW( TAG, fmt, std::forward< Args >( args )... );
        }

        template< typename ...Args > DWMQ_inline void inf( const char* fmt, Args&&... args ) {
            ESP_LOGI( TAG, fmt, std::forward< Args >( args )... );
        }

    } Echo;

public:
    struct CONFIG {
        struct IR_REMOTE {
            static constexpr ir_signal_t   signal_toggle_power    = 0xFFC23D;
            static constexpr ir_signal_t   signal_blue_enable     = 0xFF02FD;
            static constexpr ir_signal_t   signal_blue_disable    = 0xFF22DD;
            static constexpr ir_signal_t   signal_vol_up          = 0xFFA857;
            static constexpr ir_signal_t   signal_vol_down        = 0xFFE01F;

            static constexpr int           stack_main             = 4096;
            
            static constexpr int           after_signal_hold_ms   = 1000;

        } IrRemote;

        struct RELAY_GRID {
            static constexpr int   power_hold_ms   = 500; 

        } RelayGrid;

    } Config;

public:
    status_t init( void ) {
        Echo.init();

        esp_task_wdt_deinit();

        return 0;
    }

} Miruna;


};
