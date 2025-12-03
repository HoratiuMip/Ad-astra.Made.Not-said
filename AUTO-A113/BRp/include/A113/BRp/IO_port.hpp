#pragma once
/**
 * @file: BRp/IO_port.hpp
 * @brief: Basic input/output interface.
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/descriptor.hpp>

namespace a113 { namespace io {


typedef   uint32_t   ipv4_addr_t;
typedef   uint16_t   ipv4_port_t;


class Port {
public:
    virtual status_t read( const MDsc& mdsc_ ) = 0;

    virtual status_t write( const MDsc& mdsc_ ) = 0;

public:
    status_t basic_read_loop( const MDsc& mdsc_ );

    status_t basic_write_loop( const MDsc& mdsc_ );
};


} };
