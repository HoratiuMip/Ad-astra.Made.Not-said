#pragma once
/*===== IXN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Driver for the BarraCUDA controller.
|
======*/

#include <IXN/descriptor.hpp>
#include <IXN/comms.hpp>
#include <IXN/network.hpp>


#include "../../../../Devices/BarraCUDA-CTRL/barracuda-ctrl.hpp"

#define WJP_ENVIRONMENT_MINGW
#define WJP_ARCHITECTURE_LITTLE
#include "../../../../WJP/wjp.hpp"



namespace _ENGINE_NAMESPACE { namespace _ENGINE_DEVICE_NAMESPACE {



enum BARRACUDA_CTRL_FLAG : DWORD {
    BARRACUDA_CTRL_FLAG_TRUST_INVOKER = 1 << 0,

    _BARRACUDA_CTRL_FLAG_FORCE_DWORD = 0x7f'ff'ff'ff
};

class BarracudaCTRL : public BTH_SOCKET, public WJP_DEVICE {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BarracudaCTRL" );

public:
    barcud_ctrl::dynamic_t     dynamic                = {};

_ENGINE_PROTECTED:
    bool                       _trust_invk            = false;
    std::atomic_int16_t        _wjp_seq               = { 0 };
    WJP_IBRSTBL_ENTRY          _brstbl_entry          = { dst: &dynamic, sz: sizeof( dynamic ) };
    char                       _wjp_recv_buf[ 256 ];

public:
    BarracudaCTRL& bind( void ) {
        this->WJP_DEVICE::bind_ibrstbl( WJP_IBRSTBL{
            entries: &_brstbl_entry,
            count: 1
        } );
        this->WJP_DEVICE::bind_srwrap( WJP_SRWRAP{
            send: [ this ] WJP_SEND_LAMBDA { return this->BTH_SOCKET::itr_send( src.addr, src.sz, flags ); },
            recv: [ this ] WJP_RECV_LAMBDA { return this->BTH_SOCKET::itr_recv( dst.addr, dst.sz, flags ); }
        } );
        this->WJP_DEVICE::bind_seq_acq( [ this ] () -> int16_t { return _wjp_seq.fetch_add( 1, std::memory_order_relaxed ); } );
        this->WJP_DEVICE::bind_recv_buf( WJP_BUFFER{
            addr: _wjp_recv_buf,
            sz: sizeof( _wjp_recv_buf )
        } );

        return *this;
    }

public:
    DWORD connect( DWORD flags, _ENGINE_COMMS_ECHO_ARG ) {
        _trust_invk = flags & BARRACUDA_CTRL_FLAG_TRUST_INVOKER;

        _wjp_seq.store( 0, std::memory_order_relaxed );

        DWORD ret = this->BTH_SOCKET::connect( barcud_ctrl::DEVICE_NAME_W );
        IXN_ASSERT( ret == 0, ret );
        
        return 0;
    }

    DWORD disconnect( DWORD flags, _ENGINE_COMMS_ECHO_ARG ) {
        return this->BTH_SOCKET::disconnect();
    }

public:
    DWORD ping( _ENGINE_COMMS_ECHO_ARG ) {
        echo( this, EchoLevel_Pending, "Pinging..." );

        WJP_WAIT_BACK_INFO info;
        int ret = this->wait_back( 
            &info, WJPOp_Ping, 
            WJP_CBUFFER{ addr: nullptr, sz: 0 }, 
            WJP_BUFFER{ addr: nullptr, sz: 0 }, 
            WJPSendMethod_Direct 
        );
        info.resolved.wait( false );
        
        echo( this, EchoLevel_Ok, "Received ping acknowledgement." );
        return ret;
    }

    DWORD get( std::string_view str_id, char* dest, int32_t sz, _ENGINE_COMMS_ECHO_ARG ) {
        WJP_WAIT_BACK_INFO info;

        DWORD ret = this->WJP_DEVICE::wait_back( 
            &info, WJPOp_QGet, 
            WJP_CBUFFER{ addr: str_id.data(), sz: ( int )str_id.length() + 1 }, 
            WJP_BUFFER{ addr: dest, sz: sz },
            WJPSendMethod_Direct 
        );
        IXN_ASSERT_ET( ret == 0, ret, WJP_err_strs[ info.err ] );

        info.resolved.wait( false );

        IXN_ASSERT_ET( info.ackd(), -1, info.nakr );
        return 0;
    }

    DWORD set( std::string_view str_id, const char* src, int32_t sz, _ENGINE_COMMS_ECHO_ARG ) {
        char buffer[ str_id.length() + 1 + sz ];
        strcpy( buffer, str_id.data() );
        memcpy( buffer + str_id.length() + 1, src, sz );

        WJP_WAIT_BACK_INFO info;

        DWORD ret = this->WJP_DEVICE::wait_back( 
            &info, WJPOp_QSet, 
            WJP_CBUFFER{ addr: buffer, sz: ( int )str_id.length() + 1 + sz }, 
            WJP_BUFFER{ addr: nullptr, sz: 0 },
            WJPSendMethod_Direct
        );
        IXN_ASSERT_ET( ret == 0, ret, WJP_err_strs[ info.err ] );

        info.resolved.wait( false );

        IXN_ASSERT_ET( info.ackd(), -1, info.nakr );
        return 0;
    }

public: 
    DWORD trust_resolve_recv( WJP_RESOLVE_RECV_INFO* info, _ENGINE_COMMS_ECHO_ARG ) {
        DWORD ret = this->resolve_recv( info );
        IXN_ASSERT_ET( ret > 0, ret, WJP_err_strs[ info->err ] );

        if( info->nakr ) {
            echo( this, EchoLevel_Warning ) << "Responded with NAK on sequence ( " << info->recv_head._dw1.seq << " ). Reason: " << info->nakr << ".";
        }

        return ret;
    }

};



}; };
