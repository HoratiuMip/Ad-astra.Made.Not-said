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

        IXN_ASSERT_ET( crt == count, -1, "Recv overflow." ); /* This should never happen. */
        return crt;
    }

    DWORD itr_send( const void* buffer, DWORD count, DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD crt = 0;

        do {
            DWORD ret = send( this->_socket, ( char* )buffer + crt, count - crt, flags );
            IXN_ASSERT_ET( ret > 0, ret, "Send fault. WSA code ( " << WSAGetLastError() << " )." );
            crt += ret;
        } while( crt < count );

        IXN_ASSERT_ET( crt == count, -1, "Send overflow." ); /* This should never happen. */
        return count;
    }

};



class INET_SOCKET : public SOCKET {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "INET_SOCKET" );

};



class BTH_SOCKET : public SOCKET {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BTH_SOCKET" );

/*=== DECLS ===*/ public: 
    struct addr_str_t { char buf[ sizeof( BTH_ADDR ) * 3 ] = { '\0' }; /* 3 chars per byte ( 4 high bits, 4 low bits, : ), NULL terminator */ };

public:
    addr_str_t   addr_str   = {};

/*=== ADDR UTILS ===*/ public:
    addr_str_t* addr2str( addr_str_t* dst, BTH_ADDR addr ) {
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
    inline addr_str_t addr2str( BTH_ADDR addr ) {
        addr_str_t ret = {}; this->addr2str( &ret, addr ); return ret;
    }

    DWORD dev_name2addr( BTH_ADDR* addr, std::wstring_view name, addr_str_t* out_addr_str, _ENGINE_COMMS_ECHO_RT_ARG ) {
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
        IXN_ASSERT_ET( bt_dev_find != NULL, -1, "No bluetooth devices paired to this machine." );

        do {
            if( name != bt_dev_info.szName ) continue;
            
            *addr = bt_dev_info.Address.ullLong;

            echo( this, EchoLevel_Ok ) << "Bluetooth device \"" << std::string{ name.begin(), name.end() } << "\" found at " << ( ( out_addr_str != nullptr ) ? *this->addr2str( out_addr_str, *addr ) : this->addr2str( *addr ) ).buf << "."; 
            return 0;

        } while( BluetoothFindNextDevice( bt_dev_find, &bt_dev_info ) );

        echo( this, EchoLevel_Error ) << "Bluetooth device \"" << std::string{ name.begin(), name.end() } << "\" is not paired with this machine.";
        return -1;
    }
    inline BTH_ADDR dev_name2addr( std::wstring_view name, addr_str_t* addr_str, _ENGINE_COMMS_ECHO_RT_ARG ) {
        BTH_ADDR ret = {}; this->dev_name2addr( &ret, name, addr_str, echo ); return ret;
    }

/*=== CONNECT ===*/ public:
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

        DWORD ret = this->dev_name2addr( &addr, name, &this->addr_str, echo );
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
