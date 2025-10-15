#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "core.hpp"

RP_NAMESPACE {


struct RING_MECH {
    virtual STATUS when_message( std::string_view content ) = 0;
};

struct RING_DRIVE {
    const struct PIN_MAP {
        PIN   i_uart_rx;
        PIN   q_uart_tx;
        PIN   q_reset;
    } _pin_map;

    inline static constexpr int   SIM_MODULE_PER_LINE_ITERATION_COUNT   = 6;
    inline static const char*     SIM_MODULE_OK_STR                     = "OK";
    inline static const char*     SIM_MODULE_ERROR_STR                  = "ERROR";
    inline static constexpr int   SIM_MODULE_CONFIG_ITERATION_COUNT     = 3;

    RING_DRIVE( const PIN_MAP& pin_map, HardwareSerial* port )
    : _pin_map{ pin_map }, _port{ port }
    {}

    HardwareSerial*   _port     = nullptr;
    RING_MECH*        _mech     = nullptr;
    TaskHandle_t      _h_main   = NULL;


    STATUS init( void ) {
        _port->begin( Miru.FLASH.Ring.SIM_MODULE_BAUD_RATE, SERIAL_8N1, _pin_map.i_uart_rx, _pin_map.q_uart_tx, false, Miru.FLASH.Ring.SIM_MODULE_TIMEOUT );

        RP_ASSERT_OR( xTaskCreate( &RING_DRIVE::_main, "RingDrive_Main", 4096, ( void* )this, TaskPriority_Current, &_h_main ) == pdPASS ) {
            return -1;
        }

        return 0;
    }

    void terminate( void ) {
        if( _h_main == NULL ) return;
        vTaskDelete( _h_main );
        _port->end();
    }


    static void _main( void* arg ) {
        auto*  that = ( RING_DRIVE* )arg;
        String line = "";

    l_reset:
        that->reset();
        vTaskDelay( Miru.FLASH.Ring.BOOT_HOLD_MS );


    l_config:
        for( int itr = 1; itr <= SIM_MODULE_CONFIG_ITERATION_COUNT; ++itr ) {
            if( that->_config_module() == 0 ) goto l_ok;
        }
        goto l_reset;

    l_ok:
    for(;;) {
        vTaskDelay( Miru.FLASH.Ring.MAIN_LOOP_DELAY_MS );
        
    l_check_uart: 
        if( !that->_port->available() ) continue;

        line = that->_port->readStringUntil( '\n' ); line.trim();

        if( line.startsWith( "+CMT:" ) ) {
            that->_mech->when_message( that->_port->readStringUntil( '\n' ).c_str() );
        }
       
        goto l_check_uart;
    } }

    STATUS _config_module( void ) {
        RP_ASSERT_OR( this->attention( "AT+CMGF=1" ) == SIM_MODULE_OK_STR ) return -1;
        RP_ASSERT_OR( this->attention( "AT+CNMI=2,2,0,0,0" ) == SIM_MODULE_OK_STR ) return -1;

        return 0;
    }

    RP_inline std::string attention( std::string_view cmd ) {
        _port->println( cmd.data() );
        
        String      line     = "";
        std::string response = SIM_MODULE_ERROR_STR;
        bool        valid    = false;

        for( int itr = 1; itr <= SIM_MODULE_PER_LINE_ITERATION_COUNT; ++itr ) { 
            line = _port->readStringUntil( '\n' ); line.trim(); 
            if( line.length() != 0 ) goto l_echo_ok; 
        }
        goto l_end; 
        
    l_echo_ok:
        if( !line.equals( cmd.data() ) ) goto l_end;

        for( int itr = 1; itr <= SIM_MODULE_PER_LINE_ITERATION_COUNT; ++itr ) {
            line = _port->readStringUntil( '\n' ); line.trim();
            if( line.length() == 0 ) { continue; }
            
            if( line.equals( SIM_MODULE_OK_STR ) ) { 
                if( response.empty() || response == SIM_MODULE_ERROR_STR ) response = SIM_MODULE_OK_STR; 
                valid = true; goto l_end; 
            }
            if( line.equals( SIM_MODULE_ERROR_STR ) ) { valid = true; goto l_end; }

            response = line.c_str();
        }

    l_end:
        return valid ? response : response;
    }


    RP_inline void reset( void ) {
        pinMode( _pin_map.q_reset, OUTPUT );
        digitalWrite( _pin_map.q_reset, LOW );
        vTaskDelay( Miru.FLASH.Ring.RESET_PIN_HOLD_MS );
        pinMode( _pin_map.q_reset, INPUT );
    }

    RP_inline void restart( void  ) {
        this->terminate(); this->init();
    }

};


};