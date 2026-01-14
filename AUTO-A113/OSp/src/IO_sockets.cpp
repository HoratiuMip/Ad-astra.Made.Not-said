/**
 * @file: osp/IO_sockets.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/IO_sockets.hpp>

namespace a113::io {


A113_IMPL_FNC status_t IPv4_TCP_socket::bind_peer( ipv4_addr_t addr_, ipv4_port_t port_ ) {
    A113_ASSERT_OR( false ==_conn.alive.load( std::memory_order_acquire ) ) {
        A113_LOGE_IO( "Binding another peer whilst alive." );
        return -0x1;
    }

    _conn.sock     = NULL_SOCKET;
    _conn.addr_str = ipv4_addr_str_t::from( addr_ );
    _conn.addr     = addr_;
    _conn.port     = port_;

    A113_LOGI_IO( "Peer bound." );
    return 0x0;
}

A113_IMPL_FNC status_t IPv4_TCP_socket::bind_peer( const char* addr_str_, ipv4_port_t port_ ) {
    return this->bind_peer( ipv4_addr_str_t::from( addr_str_ ), port_ );
}

A113_IMPL_FNC status_t IPv4_TCP_socket::uplink( void ) {
    status_t status = 0x0;
    
    socket_t sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    A113_ASSERT_OR( sock >= 0 ) {
        A113_LOGE_IO( "Bad socket descriptor." );
        return sock;
    }

    A113_ON_SCOPE_EXIT_L( [ &sock ] -> void {
        closesocket( sock );
    } );

    sockaddr_in desc = {};
    memset( &desc, 0, sizeof( sockaddr_in ) );

    desc.sin_family      = AF_INET;
    desc.sin_addr.s_addr = _conn.addr;
    desc.sin_port        = _conn.port; 
    
    A113_LOGD_IO( "Uplinking..." );
    status = ::connect( sock, ( sockaddr* )&desc, sizeof( sockaddr_in ) );
    A113_ASSERT_OR( 0x0 == status ) {
        A113_LOGE_IO( "Bad uplink // [{}].", WSAGetLastError() );
        return status;
    }

    _conn.sock     = sock;
    _conn.alive.store( true, std::memory_order_release );

    A113_LOGI_IO( "Uplinked." );

    A113_ON_SCOPE_EXIT_DROP;
    return status;
}

A113_IMPL_FNC status_t IPv4_TCP_socket::downlink( void ) {
    status_t status = 0x0;

    _conn.alive.store( false, std::memory_order_seq_cst );
    
    status = ::closesocket( std::exchange( _conn.sock, NULL_SOCKET ) );
    A113_ASSERT_OR( 0x0 == status ) {
        A113_LOGW_IO( "Bad socket close // [{}].", WSAGetLastError() );
    }

    _conn.addr = 0x0;
    _conn.addr_str.make_null(); 
    _conn.port = 0x0;

    return 0x0;
}

A113_IMPL_FNC status_t IPv4_TCP_socket::listen( void ) {
    A113_ASSERT_OR( 0x0 == _conn.addr ) {
        A113_LOGE_IO( "Listening with peer address different from 0:0:0:0." );
        return -0x1;
    }

    status_t status = 0x0;
    
    socket_t sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    A113_ASSERT_OR( sock >= 0 ) {
        A113_LOGE_IO( "Bad socket descriptor." );
        return sock;
    }

    A113_ON_SCOPE_EXIT_L( [ &sock ] -> void {
        ::closesocket( sock );
    } );

    sockaddr_in desc = {};
    memset( &desc, 0, sizeof( sockaddr_in ) );

    desc.sin_family      = AF_INET;
    desc.sin_addr.s_addr = 0x0;
    desc.sin_port        = _conn.port; 
    
    status = ::bind( sock, ( sockaddr* )&desc, sizeof( sockaddr_in ) );
    A113_ASSERT_OR( status == 0x0 ) {
        A113_LOGE_IO( "Bad socket bind // [{}].", WSAGetLastError() );
        return status;
    }

    A113_LOGI_IO( "Listening..."); 
    status = ::listen( sock, 1 );
    A113_ASSERT_OR( status == 0x0 ) {
        A113_LOGE_IO( "Bad socket listen // [{}].", WSAGetLastError() );
    }
    
    sockaddr_in in_desc = {}; 
    int         in_desc_sz = sizeof( sockaddr_in );
    memset( &in_desc, 0, sizeof( sockaddr_in ) );

    socket_t in_sock  = ::accept( sock, ( sockaddr* )&in_desc, &in_desc_sz );
    A113_ASSERT_OR( in_sock >= 0 ) {
        A113_LOGE_IO( "Bad accept // [{}].", WSAGetLastError() );
        return -0x1;
    }

    _conn.sock     = in_sock;
    _conn.addr     = in_desc.sin_addr.s_addr;
    _conn.addr_str = ipv4_addr_str_t::from( _conn.addr );
    _conn.port     = in_desc.sin_port;
    _conn.alive.store( true, std::memory_order_release );

    A113_LOGI_IO( "Accepted {}:{}.", ( char* )_conn.addr_str, _conn.port );

    return status;
}

A113_IMPL_FNC status_t IPv4_TCP_socket::read( const MDsc& mdsc_ ) {
    return ::recv( _conn.sock, mdsc_.ptr, mdsc_.n, MSG_WAITALL );
}

A113_IMPL_FNC status_t IPv4_TCP_socket::write( const MDsc& mdsc_ ) {
    return ::send( _conn.sock, mdsc_.ptr, mdsc_.n, 0 );
}


}
