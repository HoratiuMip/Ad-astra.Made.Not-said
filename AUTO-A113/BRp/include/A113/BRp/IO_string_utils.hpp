#pragma once
/**
 * @file: BRp/IO_string_utils.hpp
 * @brief: Utility function to convert between text and binary formns of IO-related types.
 * @details: The so dubbed "IO-related types" include: IPv4 addresses, Bluetooth adresses, and so forth.
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>

namespace A113 { namespace BRp {


struct ipv4_addr_str_t {
    inline static constexpr int   BUF_SIZE   = 0x4*0x3 + 0x3 + 0x1;

    char   buf[ BUF_SIZE ]   = { '\0' }; 

    void make_zero( void );

    static void from_ptr( uint32_t addr, ipv4_addr_str_t* ptr );
    static ipv4_addr_str_t from( uint32_t addr );
    static uint32_t from( const char* addr_str );

    operator char* () { return buf; }
};


} };
