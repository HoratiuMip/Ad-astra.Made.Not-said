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


namespace a113 { namespace io { 


#ifdef A113_TARGET_OS_WINDOWS
    typedef   ::SOCKET   socket_t;

    inline constexpr socket_t   NULL_SOCKET   = INVALID_SOCKET;
#endif 


class IPv4_TCP_socket : public Port, public st_att::_Log {
_A113_PROTECTED:
    struct {
        std::atomic_bool   alive      = { false };
        socket_t           sock       = NULL_SOCKET;
        ipv4_addr_str_t    addr_str   = {};
        ipv4_addr_t        addr       = 0x0;
        ipv4_port_t        port       = 0x0;
    } _conn;

public:
    A113_inline const char* addr_c_str( void ) { return _conn.addr_str.buf; }
    A113_inline ipv4_port_t port( void ) { return _conn.port; }

public:
    status_t bind_peer( ipv4_addr_t addr_, ipv4_port_t port_ );
    status_t bind_peer( const char* addr_str_, ipv4_port_t port_ );

public:
    status_t uplink( void );
    status_t downlink( void );

    status_t listen( void );

public:
    virtual status_t read( const MDsc& mdsc_ ) override;

    virtual status_t write( const MDsc& mdsc_ ) override;
};


} };



