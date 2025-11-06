#pragma once
/**
 * @file: uCp/Core.hpp
 * @brief: 
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <Arduino.h>

#include <atomic>
#include <memory>


#define RNK_inline inline
#define RNK_PROTECTED protected

#define RNK_ASSERT_OR( cond ) if( !(cond) )


namespace rnk {


template< typename _T > class Atomic : public std::atomic< _T > {};


struct MDsc { uint8_t* ptr; size_t sz; };


inline const char*   TAG   = "RNK";

typedef   uint8_t   pin_t;
typedef   int       status_t;
typedef   int16_t   pwm_t;
typedef   int64_t   time_hr_t;

enum TaskPriority_ {
    TaskPriority_Idle = tskIDLE_PRIORITY,

    TaskPriority_Perhaps,
    TaskPriority_Effect,
    TaskPriority_Current,
    TaskPriority_Urgent,
    TaskPriority_Mach
};


inline struct _LOG {
    HardwareSerial*   _port;

    template< typename ...Args > RNK_inline void error( const char* fmt, Args&&... args ) {
        _port->printf( "[%s] [error] -> ", TAG );
        _port->printf( fmt, std::forward< Args >( args )... );
        _port->println();
    }

    template< typename ...Args > RNK_inline void warn( const char* fmt, Args&&... args ) {
        _port->printf( "[%s] [warn] -> ", TAG );
        _port->printf( fmt, std::forward< Args >( args )... );
        _port->println();
    }

    template< typename ...Args > RNK_inline void info( const char* fmt, Args&&... args ) {
        _port->printf( "[%s] [info] -> ", TAG );
        _port->printf( fmt, std::forward< Args >( args )... );
        _port->println();
    }

} Log;


struct miru_begin_args_t {
    const char*       tag;
    HardwareSerial*   log_port;
    int               log_baud;
};

class _MIRU {
public:
    status_t begin( const miru_begin_args_t& args ) {
        if( args.tag ) TAG = args.tag;

        Log._port = args.log_port;
        Log._port->begin( args.log_baud );

        return 0x0;
    }

public:
    void after_failsafe( pin_t pin ) {
        vTaskSuspendAll();
        pinMode( pin, OUTPUT );
    for(;;) {
        digitalWrite( pin, 0x1 );
        vTaskDelay( 166 );
        digitalWrite( pin, 0x0 );
        vTaskDelay( 166 );
    } }

}; inline _MIRU Miru;


};