#pragma once

#include <rnk/core.hpp>
using namespace rnk;

#define   Uart                           Serial

#define   TAG                            "[ WARC-RIG-3000 ]"

#define   MODBUS_RIG_DEVICE_ID           0x3
#define   MODBUS_UART_BAUD_RATE          115200
#define   MODBUS_UART_CONFIG             SERIAL_8N1
#define   MODBUS_SERVER_BEGIN_ATTEMPTS   5
#define   MODBUS_SERVER_REATTEMPT_MS     1000_ms2t
#define   MODBUS_HOLDING_COUNT           1
#define   MODBUS_MAIN_TASK_NAME          "Modbus-main"
#define   MODBUS_MAIN_TASK_STACK_DEPTH   0x2000

#define   GPS_DEVICE_UART_BAUD_RATE      9600
#define   GPS_DEVICE_UART_CONFIG         SERIAL_8N1
#define   GPS_MAIN_TASK_NAME             "GPS-main"
#define   GPS_MAIN_TASK_STACK_DEPTH      0x2000
#define   GPS_MODBUS_HOLDING_ADDR        0x00
#define   GPS_MODBUS_HOLDING_COUNT       1

static_assert( MODBUS_HOLDING_COUNT == 
    GPS_MODBUS_HOLDING_COUNT
);
