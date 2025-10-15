#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#define RP_NAMESPACE namespace rp


RP_NAMESPACE {


inline const char* const   TAG   = "[Runik-Patriot]";


typedef   uint8_t   PIN;
typedef   int       STATUS;
typedef   int16_t   PWM;
typedef   int64_t   TIME_HR;

enum TaskPriority_ {
    TaskPriority_Idle = tskIDLE_PRIORITY,

    TaskPriority_WhenFeelingLikeIt,
    TaskPriority_Effect,
    TaskPriority_Current,
    TaskPriority_Urgent
};


enum EchoLevel_ {
    EchoLevel_Error,
    EchoLevel_Warning,
    EchoLevel_Info,
    EchoLevel_Debug
};


};


#define RP_inline inline

#define RP_ASSERT_OR( c ) if( !(c) )

#define RP_YIELD_CORE taskYIELD()


#include <string>
#include <tuple>
#include <atomic>

#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <BluetoothSerial.h>

