/**
 * @file: OSp/IO_sockets.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/IO_sockets.hpp>

namespace A113 { namespace OSp {


A113_OS_FNC RESULT IPv4_TCP_socket::uplink( BRp::ipv4_addr_t addr_, BRp::ipv4_port_t port_ ) {
    RESULT result = 0x0;

    auto addr_str = BRp::ipv4_addr_str_t::from( addr_ );
    
    socket_t sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    A113_ASSERT_OR( sock >= 0 ) {
        Log.IO->error( "Descriptor {}:{}.", ( char* )addr_str, port_ );
        return sock;
    }

    ON_FNC_EXIT on_exit{ [ &sock ] -> void {
        closesocket( sock );
    } };

    sockaddr_in desc = {};
    memset( &desc, 0, sizeof( sockaddr_in ) );

    desc.sin_family      = AF_INET;
    desc.sin_addr.s_addr = addr_;
    desc.sin_port        = port_; 
    
    Log.IO->debug( "Uplinking {}:{}...", ( char* )addr_str, port_ );
    result = ::connect( sock, ( sockaddr* )&desc, sizeof( sockaddr_in ) );
    A113_ASSERT_OR( result == 0x0 ) {
        Log.IO->error( "Uplink {}:{} // [{}].", ( char* )addr_str, port_, WSAGetLastError() );
        return result;
    }

    _conn.sock     = sock;
    _conn.addr_str = ( BRp::ipv4_addr_str_t&& )addr_str;
    _conn.addr     = addr_;
    _conn.port     = port_;
    _conn.alive.store( true, std::memory_order_release );

    Log.IO->info( "Uplinked {}:{}.", ( char* )addr_str, port_ );

    on_exit.drop();
    return result;
}

A113_OS_FNC RESULT IPv4_TCP_socket::uplink( const char* addr_str_, BRp::ipv4_port_t port_ ) {
    return this->uplink( BRp::ipv4_addr_str_t::from( addr_str_ ), port_ );
}

A113_OS_FNC RESULT IPv4_TCP_socket::downlink( void ) {
    RESULT result = 0x0;

    _conn.alive.store( false, std::memory_order_seq_cst );
    
    result = ::closesocket( std::exchange( _conn.sock, NULL_SOCKET ) );
    A113_ASSERT_OR( result == 0 ) {
        Log.IO->warn( "Close // [{}].", WSAGetLastError() );
    }

    _conn.addr = 0x0;
    _conn.addr_str.make_null(); 
    _conn.port = 0x0;

    return 0x0;
}

A113_OS_FNC RESULT IPv4_TCP_socket::listen( BRp::ipv4_port_t port_ ) {
    RESULT result = 0x0;
    
    socket_t sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    A113_ASSERT_OR( sock >= 0 ) {
        Log.IO->error( "Descriptor {}.", port_ );
        return sock;
    }

    ON_FNC_EXIT on_exit{ [ &sock ] -> void {
        ::closesocket( sock );
    } };

    sockaddr_in desc = {};
    memset( &desc, 0, sizeof( sockaddr_in ) );

    desc.sin_family      = AF_INET;
    desc.sin_addr.s_addr = 0x0;
    desc.sin_port        = port_; 
    
    result = ::bind( sock, ( sockaddr* )&desc, sizeof( sockaddr_in ) );
    A113_ASSERT_OR( result == 0x0 ) {
        Log.IO->error( "Bind {} // [{}].", port_, WSAGetLastError() );
        return result;
    }

    Log.IO->info( "Listening {}...", port_ ); 
    result = ::listen( sock, 1 );
    A113_ASSERT_OR( result == 0x0 ) {
        Log.IO->error( "Listen {} // [{}].", port_, WSAGetLastError() );
    }
    
    sockaddr_in in_desc = {}; 
    int         in_desc_sz = sizeof( sockaddr_in );
    memset( &in_desc, 0, sizeof( sockaddr_in ) );

    socket_t in_sock  = ::accept( sock, ( sockaddr* )&in_desc, &in_desc_sz );
    A113_ASSERT_OR( in_sock >= 0 ) {
        Log.IO->error( "Accept {} // [{}].", port_, WSAGetLastError() );
        return -0x1;
    }

    _conn.sock     = in_sock;
    _conn.addr     = in_desc.sin_addr.s_addr;
    _conn.addr_str = BRp::ipv4_addr_str_t::from( _conn.addr );
    _conn.port     = port_;
    _conn.alive.store( true, std::memory_order_release );

    Log.IO->info( "Accepted {}:{}.", ( char* )_conn.addr_str, port_ );

    return result;
}

A113_OS_FNC RESULT IPv4_TCP_socket::read( const BUFFER& buf ) {
    return ::recv( _conn.sock, buf.ptr, buf.n, MSG_WAITALL );
}

A113_OS_FNC RESULT IPv4_TCP_socket::write( const BUFFER& buf ) {
    return ::send( _conn.sock, buf.ptr, buf.n, 0 );
}


} };
