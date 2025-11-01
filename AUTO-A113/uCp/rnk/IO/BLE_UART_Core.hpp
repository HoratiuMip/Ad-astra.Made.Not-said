#pragma once
/**
 * @file: uCp/IO/BLE_UART_Core.hpp
 * @brief: 
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "../Core.hpp"
#include <NimBLEDevice.h>

namespace rnk { namespace IO { namespace BLE_UART {

inline const struct _UUID {
    const NimBLEUUID   SERVICE { "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" };
    const NimBLEUUID   CHAR_M2H{ "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" };
    const NimBLEUUID   CHAR_H2M{ "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" };
} UUID;

} } };