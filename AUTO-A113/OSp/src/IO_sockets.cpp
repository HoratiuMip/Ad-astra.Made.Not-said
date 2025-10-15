/**
 * @file: OSp/IO_sockets.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/IO_sockets.hpp>

namespace A113 { namespace OSp {


A113_OS_FNC RESULT IPv4_TCP_socket::connect( uint32_t addr, uint16_t port ) {
    RESULT result = 0x0;

    auto addr_str = BRp::ipv4_addr_str_t::from( addr );
    
    socket_t sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    A113_ASSERT_OR( sock >= 0 ) {
        Log->error( "Flawed creation of socket descriptor for connecting to {}:{}.", ( char* )addr_str, port );
        return sock;
    }

    ON_FNC_EXIT on_exit{ [ &sock ] -> void {
        closesocket( sock );
    } };

    sockaddr_in desc = {};
    memset( &desc, 0, sizeof( sockaddr_in ) );

    desc.sin_family      = AF_INET;
    desc.sin_addr.s_addr = addr;
    desc.sin_port        = port; 
    
    Log->info( "Connecting to {:s}:{}...", ( char* )addr_str, port );
    result = ::connect( sock, ( sockaddr* )&desc, sizeof( sockaddr_in ) );
    A113_ASSERT_OR( result == 0x0 ) {
        Log->error( "Flawed connection to {}:{}, [{}].", ( char* )addr_str, port, WSAGetLastError() );
        return result;
    }

    _conn.sock     = sock;
    _conn.addr_str = ( BRp::ipv4_addr_str_t&& )addr_str;
    _conn.addr     = addr;
    _conn.port     = port;
    _conn.alive.store( true, std::memory_order_release );

    Log->info( "Connected to {:s}:{}.", ( char* )addr_str, port );

    on_exit.drop();
    return result;
}

A113_OS_FNC RESULT IPv4_TCP_socket::connect( const char* addr_str, uint16_t port ) {
    return this->connect( BRp::ipv4_addr_str_t::from( addr_str ), port );
}

A113_OS_FNC RESULT IPv4_TCP_socket::listen( uint16_t port ) {
    RESULT result = 0x0;
    
    socket_t sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    A113_ASSERT_OR( sock >= 0 ) {
        Log->error( "Flawed creation socket descriptor for listening on port {}.", port );
        return sock;
    }

    ON_FNC_EXIT on_exit{ [ &sock ] -> void {
        closesocket( sock );
    } };

    sockaddr_in desc = {};
    memset( &desc, 0, sizeof( sockaddr_in ) );

    desc.sin_family      = AF_INET;
    desc.sin_addr.s_addr = 0x0;
    desc.sin_port        = port; 
    
    result = ::bind( sock, ( sockaddr* )&desc, sizeof( sockaddr_in ) );
    A113_ASSERT_OR( result == 0x0 ) {
        Log->error( "Flawed binding of the socket descriptor for listening on port {}, [{}]", port, WSAGetLastError() );
        return result;
    }

    Log->info( "Listening on port {}...", port );
    result = ::listen( sock, SOMAXCONN );
    A113_ASSERT_OR( result == 0x0 ) {
        Log->error( "Flawed listen on port {}, [{}].", port, WSAGetLastError() );
    }
    
    sockaddr_in in_desc = {}; 
    int         in_desc_sz = sizeof( sockaddr_in );
    memset( &in_desc, 0, sizeof( sockaddr_in ) );

    socket_t in_sock  = ::accept( sock, ( sockaddr* )&in_desc, &in_desc_sz );
    A113_ASSERT_OR( in_sock >= 0 ) {
        Log->error( "Flawed acceptance of connection on port {}, [{}].", port, WSAGetLastError() );
        return -0x1;
    }

    _conn.sock     = in_sock;
    _conn.addr     = in_desc.sin_addr.s_addr;
    _conn.addr_str = BRp::ipv4_addr_str_t::from( _conn.addr );
    _conn.port     = port;
    _conn.alive.store( true, std::memory_order_release );

    Log->info( "Accepted connection on {}:{}.", ( char* )_conn.addr_str, port );

    return result;
}

A113_OS_FNC RESULT IPv4_TCP_socket::read( const BUFFER& buf ) {
    return ::recv( _conn.sock, buf.ptr, buf.n, 0 );
}

A113_OS_FNC RESULT IPv4_TCP_socket::write( const BUFFER& buf ) {
    return ::send( _conn.sock, buf.ptr, buf.n, 0 );
}


} };
