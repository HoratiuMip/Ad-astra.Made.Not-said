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

class BarracudaCTRL : public BTH_SOCKET, public BAR_PROTO_STREAM< 256 > {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BarracudaCTRL" );

_ENGINE_PROTECTED:
    bool                       _trust_invk   = false;
  
    std::atomic_int32_t        _bar_seq      = { 0 };

public:
    DWORD connect( DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        _trust_invk = flags & BARRACUDA_CTRL_FLAG_TRUST_INVOKER;

        DWORD ret = this->BTH_SOCKET::connect( bar_ctrl::DEVICE_NAME_W );
        NLN_ASSERT( ret == 0, ret );
        
        this->BAR_PROTO_STREAM::bind_srwrap( BAR_PROTO_SRWRAP{
            send: [ this ] BAR_PROTO_STREAM_SEND_LAMBDA { return this->BTH_SOCKET::itr_send( src, sz, flags ); },
            recv: [ this ] BAR_PROTO_STREAM_RECV_LAMBDA { return this->BTH_SOCKET::itr_recv( dst, sz, flags ); }
        } );
        this->BAR_PROTO_STREAM::bind_seq_acq( [ this ] () -> int32_t { return _bar_seq.fetch_add( 1, std::memory_order_relaxed ); } );
        
        return 0;
    }

public:
    DWORD ping( _ENGINE_COMMS_ECHO_RT_ARG ) {
        echo( this, ECHO_LEVEL_PENDING ) << "Pinging...";

        BAR_PROTO_STREAM_WAIT_BACK_INFO info;
        this->wait_back( &info, BAR_PROTO_OP_PING, nullptr, 0, nullptr, 0, BAR_PROTO_STREAM_SEND_METHOD_DIRECT );
        info.sig.wait( false );
        
        echo( this, ECHO_LEVEL_OK ) << "Received ping acknowledgement.";
        return 0;
    }

    DWORD get( std::string_view str_id, void* dest, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        BAR_PROTO_STREAM_WAIT_BACK_INFO info;
        this->BAR_PROTO_STREAM::wait_back( &info, BAR_PROTO_OP_GET, str_id.data(), str_id.length() + 1, dest, sz, BAR_PROTO_STREAM_SEND_METHOD_COPY_ON_STACK );
        info.sig.wait( false );
        return 0;
    }

    DWORD set( std::string_view str_id, void* src, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        // this->atomic_acquire_seq();
        // head->_dw0.op = BAR_PROTO_OP_SET;
        // head->_dw2.sz = str_id.length() + 1 + sz;

        // strcpy( ( char* )data, str_id.data() );
        // memcpy( ( char* )data + str_id.length() + 1, src, sz );

        // this->_emplace_resolver_and_out_cache_write( nullptr, 0, echo )->wait( false );

        // return 0;
    }

public: 
    DWORD listen_trust( bar_ctrl::dynamic_state_t* dy_st, _ENGINE_COMMS_ECHO_RT_ARG ) {
    l_listen_begin: {
        BAR_PROTO_STREAM_RESOLVE_RECV_INFO info;
        std::cout << this->resolve_recv( &info ) << '\n';

        if( info.recv_head._dw0.op != BAR_PROTO_OP_BURST ) goto l_listen_begin;
    }
        return 0;
    }

};



}; };
