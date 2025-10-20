#pragma once
/**
 * @file: BRp/IO_port.hpp
 * @brief: Basic input/output interface.
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>

namespace A113 { namespace BRp {


typedef   uint32_t   ipv4_addr_t;
typedef   uint16_t   ipv4_port_t;


class IO_port {
public:
    virtual RESULT read( const BUFFER& buf_ ) = 0;

    virtual RESULT write( const BUFFER& buf_ ) = 0;

public:
    RESULT basic_read_loop( const BUFFER& buf_ );

    RESULT basic_write_loop( const BUFFER& buf_ );
};


} };
