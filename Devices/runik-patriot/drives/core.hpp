#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#define RP_NAMESPACE namespace rp

#define RP_inline inline

RP_NAMESPACE {


typedef   uint8_t   PIN;
typedef   int       STATUS;

enum TaskPriority_ {
    TaskPriority_Idle = tskIDLE_PRIORITY,

    TaskPriority_WhenFeelingLikeIt,
    TaskPriority_Effect,
    TaskPriority_Current,
    TaskPriority_Urgent
};


};


#define RP_ASSERT_OR( c ) if( !(c) )
#define RP_SETUP_ASSERT( dl, ds, c ) { \
    Serial.print( dl ); Serial.print( "... " ); \
    if( ( status = (c) ) == 0 ) { \
        Serial.println( "OK." ); \
    } else { \
        Serial.println( "ERR." ); \
    } \
}


#include <string>
#include <tuple>

#include <EEPROM.h>


RP_NAMESPACE {


inline struct MIRU {
    struct _FLASH {
        inline static constexpr int   BYTE_COUNT   = 512;

        struct {
            char   BYTE[ 4 ]   = { 'M', 'I', 'R', 'U' };
        } ThisControl;

        struct {
            int32_t   SIM_MODULE_BAUD_RATE   = 9600;
            int16_t   SIM_MODULE_TIMEOUT     = 5000;
            int16_t   MAIN_LOOP_DELAY_MS     = 1000;
            int16_t   RESET_PIN_HOLD_MS      = 2000;
            int16_t   BOOT_HOLD_MS           = 10000;
        } RingDrive;

        STATUS init( bool reset ) {
            RP_ASSERT_OR( EEPROM.begin( BYTE_COUNT ) == true ) return -1;

            if( reset ) return this->flash();

            return this->read();
        }

        STATUS read( void ) {
            EEPROM.get( 0, *this ); 

            RP_ASSERT_OR( ThisControl.BYTE[ 0 ] == 'M' && ThisControl.BYTE[ 1 ] == 'I' && ThisControl.BYTE[ 2 ] == 'R' && ThisControl.BYTE[ 3 ] == 'U' ) {
                return -1;
            }
            return 0;
        }

        STATUS flash( void ) {
            EEPROM.put( 0, *this );
            return EEPROM.commit() ? 0 : -1;
        }
        
    } FLASH;

    STATUS init( bool reset_flash ) {
        STATUS status = 0;
        status |= FLASH.init( reset_flash );
        return status;
    }

} Miru;


};
