#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core-defs.hpp"

RP_NAMESPACE {


inline struct ECHO {
    void init( void ) {
        esp_log_level_set( "*", ESP_LOG_INFO ); 
        Serial.begin( 19200 );
    }
    
    template< typename ...Args > RP_inline void err( const char* fmt, Args&&... args ) {
        ESP_LOGE( TAG, fmt, std::forward< Args >( args )... );
    }

    template< typename ...Args > RP_inline void wrn( const char* fmt, Args&&... args ) {
        ESP_LOGW( TAG, fmt, std::forward< Args >( args )... );
    }

    template< typename ...Args > RP_inline void inf( const char* fmt, Args&&... args ) {
        ESP_LOGI( TAG, fmt, std::forward< Args >( args )... );
    }

} Echo;


struct Drive {
    std::atomic_int8_t   _ref_count   = { 0 };

    bool online( void ) {
        return _ref_count.load( std::memory_order_relaxed ) > 0;
    }
};


inline struct MIRU {
    struct _FLASH {
        inline static constexpr int   BYTE_COUNT   = 512;

        struct {
            char   BYTE[ 4 ]   = { 'M', 'I', 'R', 'U' };
        } General;

        struct Track {
            [[hot]]int16_t   PWM_LIMIT   = 200;
        } Track;

        struct {
            int32_t   SIM_MODULE_BAUD_RATE   = 9600;
            int16_t   SIM_MODULE_TIMEOUT     = 5000;
            int16_t   MAIN_LOOP_DELAY_MS     = 1000;
            int16_t   RESET_PIN_HOLD_MS      = 2000;
            int16_t   BOOT_HOLD_MS           = 10000;
        } Ring;

        STATUS init( bool reset ) {
            RP_ASSERT_OR( EEPROM.begin( BYTE_COUNT ) == true ) return -1;

            if( reset ) return this->flash();

            return this->read();
        }

        STATUS read( void ) {
            EEPROM.get( 0, *this ); 

            RP_ASSERT_OR( General.BYTE[ 0 ] == 'M' && General.BYTE[ 1 ] == 'I' && General.BYTE[ 2 ] == 'R' && General.BYTE[ 3 ] == 'U' ) {
                return -1;
            }
            return 0;
        }

        STATUS flash( void ) {
            EEPROM.put( 0, *this );
            return EEPROM.commit() ? 0 : -1;
        }
        
    } FLASH;

    struct INIT_ARGS {
        bool reset_flash;
    };

    STATUS init( const INIT_ARGS& args ) {
        STATUS status = 0;

    #if RP_CONFIG_FLAG_DEINIT_INO
        status = this->_ino_deinit();
    #endif

        status = FLASH.init( args.reset_flash );

        return status;
    }

#if RP_CONFIG_FLAG_DEINIT_INO
    STATUS _ino_deinit( void ) {
        esp_task_wdt_deinit();
        return 0;
    }
#endif

} Miru;


};