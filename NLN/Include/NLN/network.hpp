#pragma once
/*===== NLN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Structures that create an interface between ISO/OSI SESSION( 5 ) and PRESENTATION( 6 ) layers.
|
======*/

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>

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
    socket_t   _socket   = {};

public:
   

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
    BTH_SOCKET() = default;

public:
    addr_str_t   addr_str   = {};

/*=== ADDR UTILS ===*/ public:
    void addr2str( addr_str_t* dst, BTH_ADDR addr ) {
        DWORD buf_idx = -1;
        for( DWORD b_idx = sizeof( addr ) - 1; b_idx > 0; --b_idx ) { /* little endian assumed */
            UBYTE& b = ( ( UBYTE* )&addr )[ b_idx ];
            
            dst->buf[ ++buf_idx ] = ( (b>>4) > 9 ) ? ( 'A' + (b>>4) - 10 ) : ( '0' + (b>>4) );
            dst->buf[ ++buf_idx ] = ( (b&0x0F) > 9 ) ? ( 'A' + (b&0x0F) - 10 ) : ( '0' + (b&0x0F) );
            dst->buf[ ++buf_idx ] = ':';
        }
        dst->buf[ buf_idx ] = '\0';
    }
    inline addr_str_t addr2str( BTH_ADDR addr ) {
        addr_str_t ret = {}; this->addr2str( &ret, addr ); return ret;
    }

    DWORD dev_name2addr( BTH_ADDR* addr, std::wstring_view name, _ENGINE_COMMS_ECHO_RT_ARG ) {
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
        if( bt_dev_find == NULL ) {
            echo( this, ECHO_LEVEL_ERROR ) << "No bluetooth devices connected to this machine. Check bluetooth status.";
            return -1;
        }

        do {
            if( name != bt_dev_info.szName ) continue;
            
            *addr = bt_dev_info.Address.ullLong;

            echo( this, ECHO_LEVEL_OK ) << "Bluetooth device \"" << std::string{ name.begin(), name.end() } << "\" found at " << this->addr2str( *addr ).buf << "."; 
            return 0;

        } while( BluetoothFindNextDevice( bt_dev_find, &bt_dev_info ) );

        echo( this, ECHO_LEVEL_ERROR ) << "Bluetooth device \"" << std::string{ name.begin(), name.end() } << "\" is not paired with this machine.";
        return -1;
    }
    inline BTH_ADDR dev_name2addr( std::wstring_view name, _ENGINE_COMMS_ECHO_RT_ARG ) {
        BTH_ADDR ret = {}; this->dev_name2addr( &ret, name, echo ); return ret;
    }

/*=== CONNECT ===*/ public:
#if defined( _ENGINE_OS_WINDOWS )
    DWORD connect( BTH_ADDR addr, _ENGINE_COMMS_ECHO_RT_ARG ) {
        this->addr2str( &addr_str, addr );

        SOCKET::_socket = socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );

        SOCKADDR_BTH sock_addr;
        memset( &sock_addr, 0, sizeof( sock_addr ) );
        sock_addr.addressFamily  = AF_BTH;
        sock_addr.serviceClassId = RFCOMM_PROTOCOL_UUID;
        sock_addr.port           = 0;
        sock_addr.btAddr         = addr;
       
        echo( this, ECHO_LEVEL_PENDING ) << "Attempting to connect to " << addr_str.buf << ".";
        if( DWORD ret = ::connect( SOCKET::_socket, ( SOCKADDR* )&sock_addr, sizeof( sock_addr ) ); ret != 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Fault connecting to " << addr_str.buf << ". Returned (" << ret << "), WSA code (" << WSAGetLastError() << ").";
            return ret;
        }

        echo( this, ECHO_LEVEL_OK ) << "Connected to " << addr_str.buf << ".";
        return 0;
    }
    inline DWORD connect( std::wstring_view name, _ENGINE_COMMS_ECHO_RT_ARG ) {
        BTH_ADDR addr; if( DWORD ret = this->dev_name2addr( &addr, name, echo ); ret != 0 ) return ret;
        return this->connect( addr, echo );
    }

    DWORD disconnect( _ENGINE_COMMS_ECHO_RT_ARG ) {
        if( SOCKET::_socket == socket_t{} ) return 0;
        
        if( DWORD ret = closesocket( SOCKET::_socket ); ret != 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Fault disconnecting from " << addr_str.buf << ". Returned (" << ret << "), WSA code (" << WSAGetLastError() << ").";
            return ret;
        }

        echo( this, ECHO_LEVEL_OK ) << "Disconnected from " << addr_str.buf << ".";

        addr_str.buf[ 0 ] = '\0';
        return 0;
    }
#endif

};



};
