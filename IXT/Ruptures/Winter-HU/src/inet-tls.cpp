#include <wnt/inet-tls.hpp>
#include <wnt/common.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <list>

namespace wnt { namespace inet_tls {


static struct _INTERNAL {
#if defined( WIN32 )    
    WSADATA                wsa_data;
#endif
    const SSL_METHOD*      ssl_method;
    SSL_CTX*               ssl_context;

    std::list< HBRIDGE >   bridge_supervisor;


    int uplink( VOID_DOUBLE_LINK vdl ) {
        int status = -1;

    #if defined( WIN32 )
        memset( &this->wsa_data, 0, sizeof( WSADATA ) );
        status = WSAStartup( MAKEWORD( 2, 2 ), &this->wsa_data );

        WNT_ASSERT( status == 0, "WSAStartup failed.", status, -1 );

        WNT_LOG_OK << "WSAStartup success.";
    #endif

        SSL_library_init(); /* Always returns 1. */
        WNT_LOG_OK << "SSL library init success.";

        SSL_load_error_strings();
        WNT_LOG_OK << "SSL load error strings success.";

        this->ssl_method  = TLS_client_method();
        this->ssl_context = SSL_CTX_new( this->ssl_method );

        SSL_CTX_set_min_proto_version( this->ssl_context, TLS1_1_VERSION );

        WNT_LOG_OK << "Uplink complete.\n";

        return 0;
    }

    int downlink( VOID_DOUBLE_LINK vdl ) {
        int status = -1;

    #if defined( WIN32 )
        status = WSACleanup();
    #endif

        for( auto& bridge : this->bridge_supervisor ) {
            SSL_shutdown( bridge->_ssl );
        }

        SSL_CTX_free( this->ssl_context );

        WNT_LOG_OK << "Downlink complete.\n";

        return 0;
    }

} _internal;


int uplink( VOID_DOUBLE_LINK vdl ) {
    return _internal.uplink( vdl );
}

int downlink( VOID_DOUBLE_LINK vdl ) {
    return _internal.downlink( vdl );
}


BRIDGE::BRIDGE( const char* addr, INET_PORT port ) {
    int          status;
    _SOCKET      socket_raw;
    sockaddr_in  socket_desc;
    char*        err;


    WNT_ASSERT( addr != nullptr, "Address is NULL.", -1, ; );
    WNT_ASSERT( port == INET_PORT_HTTPS, "As of now, port must be HTTPS.", -1, ; );


    memset( this, 0, sizeof( BRIDGE ) );

    socket_raw = socket( AF_INET, SOCK_STREAM, 0 );
    WNT_ASSERT( socket_raw >= 0, "Socket creation fault.", socket_raw, ; );

    memset( &socket_desc, 0, sizeof( sockaddr_in ) );

    socket_desc.sin_family      = AF_INET;
    socket_desc.sin_addr.s_addr = inet_addr( addr );
    socket_desc.sin_port        = htons( port ); 
    
    WNT_LOG_PENDING << "Connecting to " << addr << " on " << port << "...";
    status = connect( socket_raw, ( sockaddr* )&socket_desc, sizeof( sockaddr_in ) );
    WNT_ASSERT( status == 0, "Server connection fault.", status, ; );
    WNT_LOG_OK << "Connected.";

    this->_ssl = SSL_new( _internal.ssl_context );
    err = ERR_error_string( ERR_get_error(), 0 );
    WNT_ASSERT( this->_ssl != nullptr, err ? err : "SSL creation fault.", -1, ; );

    this->_socket = SSL_get_fd( this->_ssl );
    SSL_set_fd( this->_ssl, socket_raw );

    WNT_LOG_PENDING << "Securely connecting to " << addr << " on " << port << "...";
    status = SSL_connect( this->_ssl );
    WNT_ASSERT( status > 0, "Secure handshake fault.", -1, ; );
    WNT_LOG_OK << "Securely connected.";


    WNT_LOG_OK << "Secure socket created, using " << SSL_get_cipher( this->_ssl ) << ".\n";
}

HBRIDGE BRIDGE::alloc( const char* addr, INET_PORT port ) {
    HBRIDGE bridge = _internal.bridge_supervisor.emplace_back( std::make_shared< BRIDGE >() );

    new ( bridge.get() ) BRIDGE{ addr, port };

    return _internal.bridge_supervisor.back();
}

void BRIDGE::free() {
    return;
}

int BRIDGE::write( const char* buf, int sz ) {
    int w = -1;

    WNT_ASSERT( buf != nullptr, "NULL buffer.", -1, -1 );
    WNT_ASSERT( sz > 0, "Write count < 0.", -1, -1 );

    w = SSL_write( this->_ssl, buf, sz );

    return w;
}

std::string BRIDGE::read( int sz ) {
    int r = -1;
    
    WNT_ASSERT( sz > 0, "Read count < 0.", -1, "" );

    char buf[ sz + 1 ];
    r = SSL_read( this->_ssl, buf, sz );
    buf[ r ] = '\0';

    return buf;
}

std::string BRIDGE::xchg( const char* buf, int w_sz, int r_sz ) {
    int status = -1;

    status = this->write( buf, w_sz );

    WNT_ASSERT( status > 0, "Xchg write fault.", status, "" );

    return this->read( r_sz );
}


}; };