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

struct port_RW_desc_t {
    char*     ptr_SoD           = nullptr;
    size_t    n_SoD             = 0;
    size_t*   byte_count        = nullptr;
    bool      fail_if_not_all   = false;
};

class Port {
public:
    virtual status_t read( const port_RW_desc_t& desc_ ) = 0;

    virtual status_t write( const port_RW_desc_t& desc_ ) = 0;

public:
    status_t basic_read_loop( const port_RW_desc_t& desc_ );

    status_t basic_write_loop( const port_RW_desc_t& desc_ );
};


} };
