#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <string>
#include <tuple>
#include <atomic>

#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <esp_http_server.h>

#include <rnk/Core.hpp>

#include "../abstraction_layer.hpp"

namespace rp {


struct Drive {
    std::atomic_int8_t   _ref_count   = { 0 };

    bool online( void ) {
        return _ref_count.load( std::memory_order_relaxed ) > 0;
    }
};


struct miru_begin_args_t {
    bool reset_flash;
};

inline struct _MIRU {
    struct _FLASH {
        inline static constexpr int   BYTE_COUNT   = 512;

        struct {
            char   BYTE[ 4 ]   = { 'M', 'I', 'R', 'U' };
        } _Align;

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

        rnk::status_t begin( bool reset ) {
            RNK_ASSERT_OR( EEPROM.begin( BYTE_COUNT ) == true ) return -1;

            if( reset ) return this->flash();

            return this->read();
        }

        rnk::status_t read( void ) {
            EEPROM.get( 0, *this ); 

            RNK_ASSERT_OR( _Align.BYTE[ 0 ] == 'M' && _Align.BYTE[ 1 ] == 'I' && _Align.BYTE[ 2 ] == 'R' && _Align.BYTE[ 3 ] == 'U' ) {
                return -0x1;
            }
            return 0x0;
        }

        rnk::status_t flash( void ) {
            EEPROM.put( 0, *this );
            return EEPROM.commit() ? 0x0 : -0x1;
        }
        
    } FLASH;

    rnk::status_t begin( const miru_begin_args_t& args ) {
        rnk::status_t status = 0;

    #if RP_CONFIG_FLAG_DEINIT_INO
        status = this->_ino_deinit();
    #endif

        status = FLASH.begin( args.reset_flash );

        return status;
    }

#if RP_CONFIG_FLAG_DEINIT_INO
    rnk::status_t _ino_deinit( void ) {
        esp_task_wdt_deinit();
        return 0x0;
    }
#endif

} Miru;


};