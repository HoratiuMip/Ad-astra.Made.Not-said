#include <HU/inet-ssl.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <list>

namespace HU { namespace inet_ssl {


static struct _INTERNAL {
/* Windows sockets specifics. */
#if defined( WIN32 )     
    WSADATA                     wsa_data;
#endif
    std::list< SOCKET_ENTRY >   sockets;

    const SSL_METHOD*           method;
    SSL_CTX*                    ctx;

} _internal;


int uplink( BLIND_ARG barg ) {
    int status = -1;

/* OS specifics. */
#if defined( WIN32 )
    memset( &_internal.wsa_data, 0, sizeof( WSADATA ) );
    status = WSAStartup( MAKEWORD( 2, 2 ), &_internal.wsa_data );

    switch( status ) {
        case WSASYSNOTREADY: {
            HU_LOG_ERROR << "The underlying network subsystem is not ready for network communication. Aborted.";
        return status; }
        case WSAVERNOTSUPPORTED: {
            HU_LOG_ERROR << "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation. Aborted.";
        return status; }
        case WSAEINPROGRESS: {
            HU_LOG_ERROR << "A blocking Windows Sockets 1.1 operation is in progress. Aborted.";
        return status; }
        case WSAEPROCLIM: {
            HU_LOG_ERROR << "A limit on the number of tasks supported by the Windows Sockets implementation has been reached. Aborted.";
        return status; }
        case WSAEFAULT: {
            HU_LOG_ERROR << "The lpWSAData parameter is not a valid pointer. Aborted.";
        return status; }
    }

    HU_LOG_OK << "WSAStartup success.";
#endif

    SSL_library_init(); /* Always returns 1. */
    HU_LOG_OK << "SSL library init success.";

    SSL_load_error_strings();
    HU_LOG_OK << "SSL load error strings success.";

    _internal.method = SSLv23_client_method();
    _internal.ctx    = SSL_CTX_new( _internal.method );

    HU_LOG_OK << "inet-ssl uplink complete.\n";

    return 0;
}

int downlink( BLIND_ARG barg ) {
    int status = -1;

/* OS specifics. */
#if defined( WIN32 )
    status = WSACleanup();
#endif

    HU_LOG_OK << "inet-ssl downlink complete.\n";

    return 0;
}


SOCKET_HANDLE push_client_v4( const char* addr, uint16_t port ) {
    int             status       = -1;
    SOCKET_ENTRY    out;
    SOCKET_TYPE     sock;
    sockaddr_in     sock_desc;
    SOCKET_HANDLE   ret;


    HU_ASSERT( addr != nullptr, "Address is NULL.", {} );
    HU_ASSERT( port == HTTPS_PORT, "Port must be HTTPS.", {} );

    memset( &out, 0, sizeof( SOCKET_ENTRY ) );

    sock = socket( AF_INET, SOCK_STREAM, 0 );
    HU_ASSERT( sock >= 0, "Failed creating socket.", {} );

    memset( &sock_desc, 0, sizeof( sockaddr_in ) );

    sock_desc.sin_family      = AF_INET;
    sock_desc.sin_addr.s_addr = inet_addr( addr );
    sock_desc.sin_port        = htons( port ); 
    
    HU_LOG_PENDING << "Connecting to " << addr << " on " << port << ".";
    status = connect( sock, ( sockaddr* )&sock_desc, sizeof( sockaddr_in ) );
    HU_ASSERT( status == 0, "Failed to connect to server.", {} );

    out.ssl = SSL_new( _internal.ctx );
    HU_ASSERT( out.ssl != nullptr, ERR_error_string( ERR_get_error(), 0 ), {} );

    out.sock = SSL_get_fd( out.ssl );
    SSL_set_fd( out.ssl, sock );

    status = SSL_connect( out.ssl );
    HU_ASSERT( status > 0, "Failed to handshake a secure connection.", {} );


    _internal.sockets.emplace_back( out );

    HU_LOG_OK << "Pushed a secure socket, using " << SSL_get_cipher( out.ssl ) << ".";

    ret = _internal.sockets.end();
    --ret;

    return ret;
}


int write( SOCKET_HANDLE handle, const char* buf, int sz ) {
    int w = -1;

    HU_ASSERT( handle != SOCKET_HANDLE{}, "Cannot use a NULL handle.", -1 );
    HU_ASSERT( buf != nullptr, "Cannot write on the socket from NULL.", -1 );
    HU_ASSERT( sz > 0, "Cannot write < 0 on the socket.", -1 );

    w = SSL_write( handle->ssl, buf, sz );

    return w;
}

std::string read( SOCKET_HANDLE handle, int sz ) {
    int r = -1;
    
    HU_ASSERT( handle != SOCKET_HANDLE{}, "Cannot use a NULL handle.", "" );
    HU_ASSERT( sz > 0, "Cannot read < 0 from the socket.", "" );

    char buf[ sz + 1 ];
    r = SSL_read( handle->ssl, buf, sz );
    buf[ r ] = '\0';

    return buf;
}


}; };