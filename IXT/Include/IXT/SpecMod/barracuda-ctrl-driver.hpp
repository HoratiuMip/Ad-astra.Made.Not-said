#pragma once
/*
Special module linking the IXT engine with the BARRACUDA controller, via bluetooth, on Windows.
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

#include <IXT/SpecMod/barracuda-ctrl.hpp>

#include <Ws2bth.h>
#include <BluetoothAPIs.h>

namespace _ENGINE_NAMESPACE { namespace SpecMod {



enum BARRACUDA_CONTROLLER_FLAG : DWORD {
    BARRACUDA_CONTROLLER_FLAG_NO_BLOCK = 1 << 0,

    _BARRACUDA_CONTROLLER_FLAG_FORCE_DWORD = 0x7F'FF'FF'FF
};

class BarracudaController : public Descriptor {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "SpecMod::BarracudaController" );

public:
    BarracudaController() = default;

    ~BarracudaController() {
        if( this->uplinked() ) closesocket( std::exchange( _bt_socket, SOCKET{} ) );
    }

_ENGINE_PROTECTED:
    BTH_ADDR       _on_board_uc_bt_addr   = {};
    SOCKADDR_BTH   _bt_socket_addr        = {};
    SOCKET         _bt_socket             = {};

public:
    bool uplinked() {
        return _bt_socket != SOCKET{};
    }

public:
    DWORD data_link( const std::wstring& name, DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        if( DWORD rez = this->_query_system_load_bt_addr( name, flags, echo ); rez != 0 ) return rez;

        if( DWORD rez = this->_connect_load_socket( flags, echo ); rez != 0 ) return rez;

        return 0;
    }

_ENGINE_PROTECTED:
    DWORD _query_system_load_bt_addr( const std::wstring& name, DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        echo( this, ECHO_LEVEL_INTEL ) << "Pulling bluetooth address.";

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

        BLUETOOTH_DEVICE_INFO bt_dev_info = { sizeof( BLUETOOTH_DEVICE_INFO ), 0 };

        HBLUETOOTH_DEVICE_FIND bt_dev = BluetoothFindFirstDevice( &bt_dev_sp, &bt_dev_info );
        if( bt_dev == NULL ) {
            echo( this, ECHO_LEVEL_ERROR ) << "No bluetooth devices paired to this machine.";
            return -1;
        }

        do {
            if( name != bt_dev_info.szName ) continue;
            
            if( !bt_dev_info.fAuthenticated ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Bluetooth device found but not authenticated.";
                return -1;
            }

            _on_board_uc_bt_addr = bt_dev_info.Address.ullLong;
            break;
        } while( BluetoothFindNextDevice( bt_dev, &bt_dev_info ) );

        if( _on_board_uc_bt_addr == BTH_ADDR{} ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Fault when pulling bluetooth address.";
            return -1;
        }

        echo( this, ECHO_LEVEL_OK ) << "Bluetooth address pulled.";
        return 0;
    }

    DWORD _connect_load_socket( DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        echo( this, ECHO_LEVEL_PENDING ) << "Attempting data link...";

        _bt_socket = socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );

        memset( &_bt_socket_addr, 0, sizeof( SOCKADDR_BTH ) );
        _bt_socket_addr.addressFamily  = AF_BTH;
        _bt_socket_addr.serviceClassId = RFCOMM_PROTOCOL_UUID;
        _bt_socket_addr.port           = 0;
        _bt_socket_addr.btAddr         = _on_board_uc_bt_addr;

        if( ::connect( _bt_socket, ( SOCKADDR* )&_bt_socket_addr, sizeof( SOCKADDR_BTH ) ) != 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Fault on data link ( " << WSAGetLastError() << " ).";
            return -1;
        }

        if( !( flags & BARRACUDA_CONTROLLER_FLAG_NO_BLOCK ) ) goto l_skip_no_block;
        {
        unsigned long no_block_mode = 1;
        if( ioctlsocket( _bt_socket, FIONBIO, ( unsigned long* )&no_block_mode ) != 0 ) {                                                                          
            echo( this, ECHO_LEVEL_WARNING ) << "Fault when setting the data link socket to non-blocking mode. Program freezes might be caused by TX/RX errors/halts on this device.";
        }
        }
        l_skip_no_block:

        echo( this, ECHO_LEVEL_OK ) << "Data link complete.";
        return 0;
    }

public:
    DWORD _read( char* buffer, DWORD count, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD crt_count = 0;
        
        do {
            DWORD result = recv( _bt_socket, buffer + crt_count, count - crt_count, 0 );
            if( result <= 0 ) { 
                echo( this, ECHO_LEVEL_ERROR ) << "RX fault( " << result << " | " << WSAGetLastError() << " )."; 
                return result; 
            }
            crt_count += result;
        } while( crt_count < count );

        if( crt_count != count ) { 
            echo( this, ECHO_LEVEL_ERROR ) << "RX fault, too many bytes read."; 
            return -1; 
        }
        return count;
    }

    DWORD _write( const char* buffer, DWORD count, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD crt_count = 0;

        do {
            DWORD result = send( _bt_socket, buffer + crt_count, count - crt_count, 0 );
             if( result <= 0 ) { 
                echo( this, ECHO_LEVEL_ERROR ) << "TX fault( " << result << " |" << WSAGetLastError() << " )."; 
                return result; 
            }
            crt_count += result;
        } while( crt_count < count );

        if( crt_count != count ) { 
            echo( this, ECHO_LEVEL_ERROR ) << "TX fault, too many bytes written."; 
            return -1; 
        }

        return count;
    }

public:
    DWORD listen_for_desc( barracuda_ctrl::state_desc_t* desc, _ENGINE_COMMS_ECHO_RT_ARG ) {
        barracuda_ctrl::proto_head_t head;
        this->_read( ( char* )&head, sizeof( head ) ); 

        

        return this->read( ( char* )desc, sizeof( barracuda_ctrl::state_desc_t ), echo );
    }

};



}; };
