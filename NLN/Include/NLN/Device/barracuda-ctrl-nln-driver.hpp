#pragma once
/*===== NLN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Driver for the BarraCUDA controller.
|
======*/

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>
#include <NLN/network.hpp>


#include "../../../../Devices/BarraCUDA-CTRL/barracuda-ctrl.hpp"
#define BAR_PROTO_ARCHITECTURE_LITTLE
#define BAR_PROTO_NOTIFIABLE_ATOMICS
#include "../../../../Devices/BarraCUDA-CTRL/bar-proto.hpp"



namespace _ENGINE_NAMESPACE { namespace _ENGINE_DEVICE_NAMESPACE {



enum BARRACUDA_CTRL_FLAG : DWORD {
    BARRACUDA_CTRL_FLAG_TRUST_INVOKER = 1 << 0,

    _BARRACUDA_CTRL_FLAG_FORCE_DWORD = 0x7f'ff'ff'ff
};

class BarracudaCTRL : public BTH_SOCKET, public BAR_PROTO< 256 > {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BarracudaCTRL" );

public:
     barcud_ctrl::dynamic_t     dynamic         = {};

_ENGINE_PROTECTED:
    bool                     _trust_invk     = false;
    std::atomic_int16_t      _bar_seq        = { 0 };
    BAR_PROTO_BRSTBL_ENTRY   _brstbl_entry   = { dst: &dynamic, sz: sizeof( dynamic ) };

public:
    void bind( void ) {
        this->BAR_PROTO::bind_brstbl( BAR_PROTO_BRSTBL{
            entries: &_brstbl_entry,
            size: 1
        } );
        this->BAR_PROTO::bind_srwrap( BAR_PROTO_SRWRAP{
            send: [ this ] BAR_PROTO_SEND_LAMBDA { return this->BTH_SOCKET::itr_send( src, sz, flags ); },
            recv: [ this ] BAR_PROTO_RECV_LAMBDA { return this->BTH_SOCKET::itr_recv( dst, sz, flags ); }
        } );
        this->BAR_PROTO::bind_seq_acq( [ this ] () -> int16_t { return _bar_seq.fetch_add( 1, std::memory_order_relaxed ); } );
    }

public:
    DWORD connect( DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        _trust_invk = flags & BARRACUDA_CTRL_FLAG_TRUST_INVOKER;

        _bar_seq.store( 0, std::memory_order_relaxed );

        DWORD ret = this->BTH_SOCKET::connect( barcud_ctrl::DEVICE_NAME_W );
        NLN_ASSERT( ret == 0, ret );
        
        return 0;
    }

    DWORD disconnect( DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        return this->BTH_SOCKET::disconnect();
    }

public:
    DWORD ping( _ENGINE_COMMS_ECHO_RT_ARG ) {
        echo( this, ECHO_LEVEL_PENDING ) << "Pinging...";

        BAR_PROTO_WAIT_BACK_INFO info;
        int ret = this->wait_back( &info, BAR_PROTO_OP_PING, nullptr, 0, nullptr, 0, BAR_PROTO_SEND_METHOD_DIRECT );
        info.sig.wait( false );
        
        echo( this, ECHO_LEVEL_OK ) << "Received ping acknowledgement.";
        return ret;
    }

    DWORD get( std::string_view str_id, void* dest, int32_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        BAR_PROTO_WAIT_BACK_INFO info;

        DWORD ret = this->BAR_PROTO::wait_back( 
            &info, BAR_PROTO_OP_GET, 
            str_id.data(), str_id.length() + 1, 
            dest, sz,
            BAR_PROTO_SEND_METHOD_COPY_ON_STACK 
        );
        NLN_ASSERT_ET( ret == 0, ret, BAR_PROTO_ERR_STR[ info.err ] );

        info.sig.wait( false );

        NLN_ASSERT_ET( info.ackd, -1, info.nakr );
        return 0;
    }

    DWORD set( std::string_view str_id, void* src, int32_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        char buffer[ str_id.length() + 1 + sz ];
        strcpy( buffer, str_id.data() );
        memcpy( buffer + str_id.length() + 1, src, sz );

        BAR_PROTO_WAIT_BACK_INFO info;

        DWORD ret = this->BAR_PROTO::wait_back( 
            &info, BAR_PROTO_OP_SET, 
            buffer, str_id.length() + 1 + sz, 
            nullptr, 0,
            BAR_PROTO_SEND_METHOD_DIRECT
        );
        NLN_ASSERT_ET( ret == 0, ret, BAR_PROTO_ERR_STR[ info.err ] );

        info.sig.wait( false );

        NLN_ASSERT_ET( info.ackd, -1, info.nakr );
        return 0;
    }

public: 
    DWORD trust_resolve_recv( BAR_PROTO_RESOLVE_RECV_INFO* info, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD ret = this->resolve_recv( info );
        NLN_ASSERT_ET( ret > 0, ret, BAR_PROTO_ERR_STR[ info->err ] );

        if( info->nakr ) {
            echo( this, ECHO_LEVEL_WARNING ) << "Responded with NAK on sequence ( " << info->recv_head._dw1.seq << " ). Reason: " << info->nakr << ".";
        }

        return ret;
    }

};



}; };
