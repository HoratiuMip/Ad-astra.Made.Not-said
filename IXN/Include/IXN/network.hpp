#pragma once
/*===== IXN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Structures that create an interface between ISO/OSI SESSION( 5 ) and PRESENTATION( 6 ) layers.
|
======*/

#include <IXN/descriptor.hpp>
#include <IXN/comms.hpp>
#include <IXN/assert.hpp>

namespace _ENGINE_NAMESPACE {



#if defined( _ENGINE_OS_WINDOWS )
    typedef   ::SOCKET   socket_t;
#elif defined( _ENGINE_OS_LINUX )
    typedef   int        socket_t;
#endif

enum INET_PORT : UWORD {
    INET_PORT_UNKNWN = 0,

    INET_PORT_HTTPS = 443,

    _INET_PORT_FORCE_UWORD = 0xff'ff
};



class SOCKET : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "SOCKET" );

_ENGINE_PROTECTED:
    socket_t   _socket   = socket_t{};

public:
    bool connected() const { return _socket != socket_t{}; }

public:
    DWORD itr_recv( void* buffer, DWORD count, DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD crt = 0;
        
        do {
            DWORD ret = recv( this->_socket, ( char* )buffer + crt, count - crt, flags | MSG_WAITALL );
            IXN_ASSERT_ET( ret > 0, ret, "Recv fault. WSA code ( " << WSAGetLastError() << " )." );
            crt += ret;
        } while( crt < count );

        IXN_ASSERT_ET( crt == count, -1, "Recv malfunction." ); /* This should never happen. */
        return crt;
    }

    DWORD itr_send( const void* buffer, DWORD count, DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD crt = 0;

        do {
            DWORD ret = send( this->_socket, ( char* )buffer + crt, count - crt, flags );
            IXN_ASSERT_ET( ret > 0, ret, "Send fault. WSA code ( " << WSAGetLastError() << " )." );
            crt += ret;
        } while( crt < count );

        IXN_ASSERT_ET( crt == count, -1, "Send malfunction." ); /* This should never happen. */
        return count;
    }

};



class INET_IPv4_SOCKET : public SOCKET {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "INET_IPv4_SOCKET" );

public:
    typedef   int32_t   addr_t;

public: 
    struct addr_str_t { char buf[ sizeof( addr_t ) * 3 + 4 ] = { '\0' }; /* max 3 chars per byte, 3 dots between them, NULL terminator */ };

public:
    static addr_str_t* addr2str( addr_str_t* dst, addr_t addr ) {
        DWORD buf_idx = 0;
        for( DWORD b_idx = sizeof( addr ) - 1; b_idx > 0; --b_idx ) { /* little endian assumed */
            UBYTE& b = ( ( UBYTE* )&addr )[ b_idx ];

            itoa( ( int )b,  &dst->buf[ buf_idx ], 10 );
            buf_idx += 3; dst->buf[ buf_idx ] = '.'; ++buf_idx;
        }
        dst->buf[ buf_idx ] = '\0';
        return dst;
    }
    inline static addr_str_t addr2str( addr_t addr ) {
        addr_str_t ret = {}; addr2str( &ret, addr ); return ret;
    }

public:
    addr_str_t   addr_str   = {};

public:
    #if defined( _ENGINE_OS_WINDOWS )
        DWORD connect( addr_t addr, _ENGINE_COMMS_ECHO_RT_ARG ) {
            #if WARC_INET_TLS == 1
        int          status        = -1;
        _SOCKET      socket_raw    = _SOCKET{};
        sockaddr_in  socket_desc   = {};
        char*        err           = nullptr; 

        struct _BAD_EXIT {
            std::function< void() >   proc;
            ~_BAD_EXIT() { if( proc != nullptr ) proc(); } 
        } bad_exit{ proc: [ &, this ] () -> void {
            if( socket_raw != _SOCKET{} )
                closesocket( socket_raw );
            this->_kill_conn();
        } };

        this->_struct_name += BRIDGE::pretty( addr, port );


        WARC_ASSERT_RT_THIS( addr != nullptr, "Address is NULL.", -1, ; );
        WARC_ASSERT_RT_THIS( this->_port == INET_PORT_HTTPS, "As of now, port must be HTTPS.", -1, ; );


        this->_addr = addr;
        
        socket_raw = socket( AF_INET, SOCK_STREAM, 0 );
        WARC_ASSERT_RT_THIS( socket_raw >= 0, "Socket creation fault.", socket_raw, ; );

        memset( &socket_desc, 0, sizeof( sockaddr_in ) );

        socket_desc.sin_family      = AF_INET;
        socket_desc.sin_addr.s_addr = inet_addr( this->_addr.c_str() );
        socket_desc.sin_port        = htons( this->_port ); 
        
        WARC_ECHO_RT_THIS_PENDING << "Connecting...";
        status = connect( socket_raw, ( sockaddr* )&socket_desc, sizeof( sockaddr_in ) );
        WARC_ASSERT_RT_THIS( status == 0, "Server connection general fault.", status, ; );

        this->_ssl = SSL_new( _internal.ssl_context );
        err = ERR_error_string( ERR_get_error(), 0 );
        WARC_ASSERT_RT_THIS( this->_ssl != nullptr, err ? err : "SSL creation fault.", -1, ; );

        this->_socket = socket_raw;
        SSL_set_fd( this->_ssl, this->_socket );

        WARC_ECHO_RT_THIS_PENDING << "TLS probing...";
        status = SSL_connect( this->_ssl );
        WARC_ASSERT_RT_THIS( status > 0, "Secure handshake fault.", -1, ; );

        bad_exit.proc = nullptr;
        WARC_ECHO_RT_THIS_OK << "Secure socket created, using " << SSL_get_cipher( this->_ssl ) << ".\n";
    #endif

        return 0;
    }

    DWORD disconnect( _ENGINE_COMMS_ECHO_RT_ARG ) {
        return 0;
    }
#endif

};



class BTH_SOCKET : public SOCKET {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BTH_SOCKET" );

public: 
    struct addr_str_t { char buf[ sizeof( BTH_ADDR ) * 3 ] = { '\0' }; /* 3 chars per byte ( 4 high bits, 4 low bits, : ), NULL terminator */ };

public:
    addr_str_t   addr_str   = {};

public:
    static addr_str_t* addr2str( addr_str_t* dst, BTH_ADDR addr ) {
        DWORD buf_idx = -1;
        for( DWORD b_idx = sizeof( addr ) - 1; b_idx > 0; --b_idx ) { /* little endian assumed */
            UBYTE& b = ( ( UBYTE* )&addr )[ b_idx ];
            
            dst->buf[ ++buf_idx ] = ( (b>>4) > 9 ) ? ( 'A' + (b>>4) - 10 ) : ( '0' + (b>>4) );
            dst->buf[ ++buf_idx ] = ( (b&0x0F) > 9 ) ? ( 'A' + (b&0x0F) - 10 ) : ( '0' + (b&0x0F) );
            dst->buf[ ++buf_idx ] = ':';
        }
        dst->buf[ buf_idx ] = '\0';
        return dst;
    }
    inline static addr_str_t addr2str( BTH_ADDR addr ) {
        addr_str_t ret = {}; addr2str( &ret, addr ); return ret;
    }

    static DWORD dev_name2addr( BTH_ADDR* addr, std::wstring_view name, addr_str_t* out_addr_str ) {
        BLUETOOTH_DEVICE_SEARCH_PARAMS bt_dev_sp = {
            dwSize:               sizeof( BLUETOOTH_DEVICE_SEARCH_PARAMS ),
            fReturnAuthenticated: true,
            fReturnRemembered:    false,
            fReturnUnknown:       true,
            fReturnConnected:     true,
            fIssueInquiry:        true,
            cTimeoutMultiplier:   2,
            hRadio:               NULL
        };

        BLUETOOTH_DEVICE_INFO bt_dev_info = { 
            dwSize:  sizeof( BLUETOOTH_DEVICE_INFO ), 
            Address: 0 
        };

        HBLUETOOTH_DEVICE_FIND bt_dev_find = BluetoothFindFirstDevice( &bt_dev_sp, &bt_dev_info );
        IXN_ASSERT_C( bt_dev_find != NULL, -1, "No bluetooth devices paired to this machine." );

        do {
            if( name != bt_dev_info.szName ) continue;
            
            *addr = bt_dev_info.Address.ullLong;

            comms( EchoLevel_Ok ) << "Bluetooth device \"" << std::string{ name.begin(), name.end() } << "\" found at " << ( ( out_addr_str != nullptr ) ? *addr2str( out_addr_str, *addr ) : addr2str( *addr ) ).buf << "."; 
            return 0;

        } while( BluetoothFindNextDevice( bt_dev_find, &bt_dev_info ) );

        comms( EchoLevel_Error ) << "Bluetooth device \"" << std::string{ name.begin(), name.end() } << "\" is not paired with this machine.";
        return -1;
    }
    inline static BTH_ADDR dev_name2addr( std::wstring_view name, addr_str_t* addr_str ) {
        BTH_ADDR ret = {}; dev_name2addr( &ret, name, addr_str ); return ret;
    }

public:
#if defined( _ENGINE_OS_WINDOWS )
    DWORD connect( BTH_ADDR addr, _ENGINE_COMMS_ECHO_RT_ARG ) {
        this->addr2str( &this->addr_str, addr );

        socket_t temp_socket = ::socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );

        SOCKADDR_BTH sock_addr;
        memset( &sock_addr, 0, sizeof( sock_addr ) );
        sock_addr.addressFamily  = AF_BTH;
        sock_addr.serviceClassId = RFCOMM_PROTOCOL_UUID;
        sock_addr.port           = 0;
        sock_addr.btAddr         = addr;
       
        echo( this, EchoLevel_Pending ) << "Attempting to connect to " << this->addr_str.buf << ".";
        DWORD ret = ::connect( temp_socket, ( SOCKADDR* )&sock_addr, sizeof( sock_addr ) );
        IXN_ASSERT_ET( ret == 0, ret, "Fault connecting to " << this->addr_str.buf << ". WSA code (" << WSAGetLastError() << ")." );
        
        SOCKET::_socket = temp_socket;
        echo( this, EchoLevel_Ok ) << "Connected to " << this->addr_str.buf << ".";
        return 0;
    }
    inline DWORD connect( std::wstring_view name, _ENGINE_COMMS_ECHO_RT_ARG ) {
        BTH_ADDR addr; 

        DWORD ret = dev_name2addr( &addr, name, &this->addr_str );
        IXN_ASSERT( ret == 0, ret );

        return this->connect( addr, echo );
    }

    DWORD disconnect( _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD ret = ::closesocket( std::exchange( SOCKET::_socket, socket_t{} ) );
        IXN_ASSERT_ET( ret == 0, ret, "Fault disconnecting from " << this->addr_str.buf << ". WSA ( " << WSAGetLastError() << ")." );

        echo( this, EchoLevel_Ok ) << "Disconnected from " << this->addr_str.buf << ".";
        addr_str.buf[ 0 ] = '\0';

        return 0;
    }
#endif

};



};
