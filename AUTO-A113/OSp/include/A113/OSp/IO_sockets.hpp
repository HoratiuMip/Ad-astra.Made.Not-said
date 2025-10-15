#pragma once
/**
 * @file: OSp/IO_sockets.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/core.hpp>

#include <A113/BRp/IO_port.hpp>
#include <A113/BRp/IO_string_utils.hpp>


namespace A113 { namespace OSp {


class IPv4_TCP_socket : public BRp::IO_port {
public:
#ifdef A113_TARGET_OS_WINDOWS
    typedef   ::SOCKET   socket_t;
#endif 

A113_PROTECTED:
    struct {
        std::atomic_bool       alive      = { false };
        socket_t               sock       = {};
        BRp::ipv4_addr_str_t   addr_str   = {};
        uint32_t               addr       = 0x0;
        uint16_t               port       = 0x0;
    } _conn;

public:
    RESULT connect( uint32_t addr, uint16_t port );
    RESULT connect( const char* addr_str, uint16_t port );

public:
    RESULT listen( uint16_t port );

public:
    virtual RESULT read( const BUFFER& buf ) override;

    virtual RESULT write( const BUFFER& buf ) override;
};


} };



