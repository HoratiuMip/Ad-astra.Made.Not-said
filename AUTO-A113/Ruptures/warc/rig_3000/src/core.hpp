#pragma once

#include <rnk/core.hpp>
using namespace rnk;

#define   TAG                            "[ WARC-RIG-3000 ]"

#define   Uart                           Serial

#define   MODBUS_UART_CONFIG             SERIAL_8N1
#define   MODBUS_SERVER_BEGIN_ATTEMPTS   5
#define   MODBUS_SERVER_REATTEMPT_MS     1000_ms2t
#define   MODBUS_MAIN_TASK_NAME          "Modbus-main"
#define   MODBUS_MAIN_TASK_STACK_DEPTH   0x2000

#define   GPS_DEVICE_UART_BAUD_RATE      9600
#define   GPS_DEVICE_UART_CONFIG         SERIAL_8N1
#define   GPS_MAIN_TASK_NAME             "GPS-main"
#define   GPS_MAIN_TASK_STACK_DEPTH      0x2000

#include "../interface.hpp"
using namespace warc::rig_3000;
