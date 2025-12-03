#pragma once
/**
 * @file: BRp/IO_string_utils.hpp
 * @brief: Utility function to convert between text and binary formns of IO-related types.
 * @details: The so dubbed "IO-related types" include: IPv4 addresses, Bluetooth adresses, and so forth.
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/descriptor.hpp>
#include <a113/brp/IO_port.hpp>

namespace a113 { namespace io {


struct ipv4_addr_str_t {
    inline static constexpr int   BUF_SIZE   = 0x4*0x3 + 0x3 + 0x1;

    char   buf[ BUF_SIZE ]   = { '\0' }; 

    void make_zero( void );
    void make_null( void ) { buf[ 0 ] = { '\0' }; }

    static void from_ptr( ipv4_addr_t addr, ipv4_addr_str_t* ptr );
    static ipv4_addr_str_t from( ipv4_addr_t addr );
    static ipv4_addr_t from( const char* addr_str );

    operator char* () { return buf; }
};


} };
