#pragma once
/**
 * @file: BRp/IO_port.hpp
 * @brief: Basic input/output interface.
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>

namespace A113 { namespace BRp {


class IO_port {
public:
    virtual RESULT read( const BUFFER& buf ) = 0;

    virtual RESULT write( const BUFFER& buf ) = 0;

public:
    RESULT basic_read_loop( const BUFFER& buf );

    RESULT basic_write_loop( const BUFFER& buf );
};


} };
