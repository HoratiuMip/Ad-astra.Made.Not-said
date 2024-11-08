#include <warc/inet-tls.hpp>

namespace warc { namespace inet_tls {


static struct _INTERNAL {
#if defined( WIN32 )    
    WSADATA                wsa_data;
#endif
    const SSL_METHOD*      ssl_method;
    SSL_CTX*               ssl_context;

    std::mutex             bsup_lock;
    std::list< HBRIDGE >   bridge_supervisor;


    int uplink( VOID_DOUBLE_LINK vdl ) {
        int status = -1;

    #if defined( WIN32 )
        memset( &this->wsa_data, 0, sizeof( WSADATA ) );
        status = WSAStartup( MAKEWORD( 2, 2 ), &this->wsa_data );

        WARC_ASSERT_RT( status == 0, "WSAStartup failed.", status, -1 );

        WARC_LOG_RT_OK << "WSAStartup success.";
    #endif

        SSL_library_init(); /* Always returns 1. */
        WARC_LOG_RT_OK << "SSL library init success.";

        SSL_load_error_strings();
        WARC_LOG_RT_OK << "SSL load error strings success.";

        this->ssl_method  = TLS_client_method();
        this->ssl_context = SSL_CTX_new( this->ssl_method );

        SSL_CTX_set_min_proto_version( this->ssl_context, TLS1_1_VERSION );

        WARC_LOG_RT_OK << "Uplink complete.\n";

        return 0;
    }

    int downlink( VOID_DOUBLE_LINK vdl ) {
        int status = -1;

        int bridge_count = this->bridge_supervisor.size();
        status = this->purge_zombie_bridges();

        if( status != bridge_count )
            WARC_LOG_RT_WARNING << "( " << ( bridge_count - status ) << " ) bridge(s) outside referenced.";

        SSL_CTX_free( this->ssl_context );

    #if defined( WIN32 )
        status = WSACleanup();
    #endif

        WARC_LOG_RT_OK << "Downlink complete.\n";

        return 0;
    }


    int purge_zombie_bridges() {
        int zombie_count = 0;

        std::unique_lock lock{ bsup_lock };

        for( auto bridge = this->bridge_supervisor.begin(); bridge != this->bridge_supervisor.end(); ) {
            if( bridge->use_count() > 1 ) { ++bridge; continue; }

            ++zombie_count;
            bridge = this->bridge_supervisor.erase( bridge );
        }

        return zombie_count;
    }

} _internal;


int uplink( VOID_DOUBLE_LINK vdl ) {
    return _internal.uplink( vdl );
}

int downlink( VOID_DOUBLE_LINK vdl ) {
    return _internal.downlink( vdl );
}


const char* BRIDGE::struct_name() const { 
    return this->_struct_name.c_str();
}

BRIDGE::BRIDGE( const char* addr, INET_PORT port, IXT_COMMS_ECHO_NO_DFT_ARG ) 
: _port{ port }
{
    int          status;
    _SOCKET      socket_raw;
    sockaddr_in  socket_desc;
    char*        err;


    this->_struct_name += BRIDGE::pretty( addr, port );


    WARC_ASSERT_ACC( addr != nullptr, "Address is NULL.", -1, ; );
    WARC_ASSERT_ACC( this->_port == INET_PORT_HTTPS, "As of now, port must be HTTPS.", -1, ; );


    this->_addr = addr;
    
    socket_raw = socket( AF_INET, SOCK_STREAM, 0 );
    WARC_ASSERT_ACC( socket_raw >= 0, "Socket creation fault.", socket_raw, ; );

    memset( &socket_desc, 0, sizeof( sockaddr_in ) );

    socket_desc.sin_family      = AF_INET;
    socket_desc.sin_addr.s_addr = inet_addr( this->_addr.c_str() );
    socket_desc.sin_port        = htons( this->_port ); 
    
    WARC_LOG_ACC_PENDING << "Connecting...";
    status = connect( socket_raw, ( sockaddr* )&socket_desc, sizeof( sockaddr_in ) );
    WARC_ASSERT_ACC( status == 0, "Server connection fault.", status, ; );
    WARC_LOG_ACC_OK << "Connected.";

    this->_ssl = SSL_new( _internal.ssl_context );
    err = ERR_error_string( ERR_get_error(), 0 );
    WARC_ASSERT_ACC( this->_ssl != nullptr, err ? err : "SSL creation fault.", -1, ; );

    this->_socket = socket_raw;
    SSL_set_fd( this->_ssl, this->_socket );

    WARC_LOG_ACC_PENDING << "Securely connecting...";
    status = SSL_connect( this->_ssl );
    WARC_ASSERT_ACC( status > 0, "Secure handshake fault.", -1, ; );
    WARC_LOG_ACC_OK << "Securely connected.";


    WARC_LOG_ACC_OK << "Secure socket created, using " << SSL_get_cipher( this->_ssl ) << ".\n";
}

BRIDGE::~BRIDGE() {
    int status = -1;

    status = SSL_shutdown( this->_ssl );
    SSL_free( this->_ssl );
    status = closesocket( this->_socket );
}

std::string BRIDGE::pretty( const char* addr, INET_PORT port ) {
    addr = addr ? addr : "NULL";

    char buf[ 6 ];
    memset( buf, 0, sizeof( buf ) );

    itoa( port, buf, 10 );

    buf[ sizeof( buf ) - 1 ] = '\0';

    return std::string{ '[' } + addr + ':' + buf + ']'; 
}

HBRIDGE BRIDGE::alloc( const char* addr, INET_PORT port ) {
    HBRIDGE bridge = _internal.bridge_supervisor.emplace_back( std::make_shared< BRIDGE >() );

    new ( bridge.get() ) BRIDGE{ addr, port };

    return bridge;
}

void BRIDGE::free( HBRIDGE&& handle ) {
    handle.reset();
    _internal.purge_zombie_bridges();
}

int BRIDGE::write( const char* buf, int sz ) {
    int w = -1;

    WARC_ASSERT_RT( buf != nullptr, "NULL buffer.", -1, -1 );
    WARC_ASSERT_RT( sz > 0, "Write count < 0.", -1, -1 );

    w = SSL_write( this->_ssl, buf, sz );

    return w;
}

std::string BRIDGE::read( int sz ) {
    int r = -1;
    
    WARC_ASSERT_RT( sz > 0, "Read count < 0.", -1, "" );

    char buf[ sz + 1 ];
    r = SSL_read( this->_ssl, buf, sz );
    buf[ r ] = '\0';

    return buf;
}

std::string BRIDGE::xchg( const char* buf, int w_sz, int r_sz ) {
    int status = -1;

    status = this->write( buf, w_sz );

    WARC_ASSERT_RT( status > 0, "Xchg write fault.", status, "" );

    return this->read( r_sz );
}


}; };