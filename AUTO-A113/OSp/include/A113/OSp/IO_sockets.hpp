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


#ifdef A113_TARGET_OS_WINDOWS
    typedef   ::SOCKET   socket_t;

    inline constexpr socket_t   NULL_SOCKET   = INVALID_SOCKET;
#endif 


class IPv4_TCP_socket : public BRp::IO_port {
A113_PROTECTED:
    struct {
        std::atomic_bool       alive      = { false };
        socket_t               sock       = NULL_SOCKET;
        BRp::ipv4_addr_str_t   addr_str   = {};
        BRp::ipv4_addr_t       addr       = 0x0;
        BRp::ipv4_port_t       port       = 0x0;
    } _conn;

public:
    A113_inline const char* addr_c_str( void ) { return _conn.addr_str.buf; }
    A113_inline BRp::ipv4_port_t port( void ) { return _conn.port; }

public:
    RESULT uplink( BRp::ipv4_addr_t addr_, BRp::ipv4_port_t port_ );
    RESULT uplink( const char* addr_str_, BRp::ipv4_port_t port_ );

    RESULT downlink( void );

public:
    RESULT listen( BRp::ipv4_port_t port_ );

public:
    virtual RESULT read( const BUFFER& buf_ ) override;

    virtual RESULT write( const BUFFER& buf_ ) override;
};


} };



