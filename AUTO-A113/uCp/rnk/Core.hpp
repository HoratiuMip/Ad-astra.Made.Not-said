#pragma once
/**
 * @file: uCp/Core.hpp
 * @brief: 
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <Arduino.h>

#include <driver/gpio.h>
#include <esp_task_wdt.h>

#include <atomic>


#define RNK_inline    inline
#define RNK_PROTECTED protected

#define RNK_ASSERT_OR( cond ) if( !(cond) )


namespace rnk {


template< typename _T > class Atomic : public std::atomic< _T > {
public: using std::atomic< _T >::atomic;
};


struct MDsc { uint8_t* ptr; size_t sz; };


typedef   gpio_num_t   pin_t;
typedef   int          status_t;
typedef   int16_t      pwm_t;
typedef   int64_t      time_hr_t;

enum TaskPriority_ {
    TaskPriority_Idle = tskIDLE_PRIORITY,

    TaskPriority_Perhaps,
    TaskPriority_Effect,
    TaskPriority_Current,
    TaskPriority_Urgent,
    TaskPriority_Mach
};


constexpr gpio_num_t operator"" _pin( unsigned long long int pin_ ) {
    return ( gpio_num_t )pin_;
}

constexpr TickType_t operator"" _ms2t( unsigned long long int ms_ ) {
    return pdMS_TO_TICKS( ms_ );
}


namespace core {
    struct pin_map_t {
       pin_t   Q_seppuku; 
    } pin_map;

    struct begin_args_t {
        pin_map_t   pin_map;
    };

    status_t begin( const begin_args_t& args_ ) {
        pin_map = args_.pin_map;

        disableCore0WDT();

        return 0x0;
    }

    void seppuku( void ) {
        vTaskSuspendAll();
    
        if( pin_map.Q_seppuku ) gpio_set_direction( pin_map.Q_seppuku, GPIO_MODE_OUTPUT );

    for( int n = 0; true ; ++n ) {
        if( not pin_map.Q_seppuku ) { vTaskDelay( 1000_ms2t ); continue; }
        gpio_set_level( pin_map.Q_seppuku, HIGH ); vTaskDelay( 100_ms2t );
        gpio_set_level( pin_map.Q_seppuku, LOW ); vTaskDelay( 100_ms2t );
        if( 0 == n % 6 ) vTaskDelay( 1000_ms2t );
    } }

};


};