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
#include "../../../../Devices/BarraCUDA-CTRL/bar-proto.hpp"



namespace _ENGINE_NAMESPACE { namespace _ENGINE_DEVICE_NAMESPACE {



enum BARRACUDA_CTRL_FLAG : DWORD {
    BARRACUDA_CTRL_FLAG_TRUST_INVOKER = 1 << 0,

    _BARRACUDA_CTRL_FLAG_FORCE_DWORD = 0x7f'ff'ff'ff
};

class BARRACUDA_CTRL : public BTH_SOCKET, public bar_cache_t< 128 > {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BARRACUDA_CTRL" );

public:
    struct resolver_t {
        int32_t            seq;
        void*              dest;
        int16_t            sz;
        std::atomic_bool   signal;
    };

_ENGINE_PROTECTED:
    std::thread                _resolver              = {};
    bool                       _trust_invk            = false;
    std::deque< resolver_t >   _resolvers             = {};
    std::mutex                 _resolvers_mtx         = {}; 
    std::atomic_int32_t        _seq                   = { 0 };

public:
    DWORD connect( DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        _trust_invk = flags & BARRACUDA_CTRL_FLAG_TRUST_INVOKER;

        DWORD ret = this->BTH_SOCKET::connect( bar_ctrl::DEVICE_NAME_W );
        NLN_ASSERT( ret == 0, ret );
        
        return 0;
    }

_ENGINE_PROTECTED:
    resolver_t& _emplace_resolver( int32_t seq, void* dest, DWORD sz ) {
        return _resolvers.emplace_back( seq, dest, sz, false );
    }

    void _atomic_resolvers_pop_front() {
        std::unique_lock lock{ _resolvers_mtx };
        _resolvers.pop_front();
    }

_ENGINE_PROTECTED:
    void atomic_acquire_seq( void ) {
        head->_dw1.seq = _seq.fetch_add( 1, std::memory_order_release );
    }

_ENGINE_PROTECTED:
    int _out_cache_write( void ) {
        return this->BTH_SOCKET::itr_send( _buffer, sizeof( *head ) + head->_dw2.sz );
    }

    std::atomic_bool* _emplace_resolver_and_out_cache_write( void* dest, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG  ) {
        std::unique_lock lock{ _resolvers_mtx };

        auto& resolver = this->_emplace_resolver( head->_dw1.seq, dest, sz );
        
        if( DWORD rez = this->_out_cache_write(); rez <= 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Cache write fault on sequence ( " << head->_dw1.seq << " ).";
            this->_resolvers.pop_back();
            return nullptr;
        }

        return &resolver.signal;
    }

    bar_proto_head_t _listen_head( _ENGINE_COMMS_ECHO_RT_ARG ) {
        bar_proto_head_t head;
        if( this->BTH_SOCKET::itr_recv( ( char* )&head, sizeof( head ) ) <= 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Head read fault.";
            return bar_proto_head_t{};
        } 
        return head;
    }

    DWORD _resolve_head( bar_proto_head_t* head, void* direct, _ENGINE_COMMS_ECHO_RT_ARG ) {
        if( !head->is_signed() ) { 
            echo( this, ECHO_LEVEL_ERROR ) << "Bad head signature ( " << ( head->sig & BAR_PROTO_SIG_MSK ) << ")."; 
            return -1; 
        }
    
        switch( head->_dw0.op ) {
            case BAR_PROTO_OP_NULL: {
                echo( this, ECHO_LEVEL_ERROR ) << "Cannot resolve head with NULL operation code.";
                return -1;
            }

            case BAR_PROTO_OP_ACK: {
                if( _resolvers.empty() ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), but not resolver.";
                    return -1;
                }

                auto& resolver = _resolvers.front();

                if( head->_dw1.seq != resolver.seq ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), but resolver expects sequence ( " << resolver.seq << " ).";
                    return -1;
                }

                if( resolver.dest == nullptr ) goto l_signal;

                if( resolver.sz != head->_dw2.sz ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), reports a different size ( " << head->_dw2.sz << " ) compared to the one the resolver expects ( " << resolver.sz << " ).";
                    return -1;
                }

                if( this->BTH_SOCKET::itr_recv( ( char* )resolver.dest, head->_dw2.sz, echo ) <= 0 ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), fault on data read.";
                    return -1;
                }

            l_signal:
                this->_atomic_resolvers_pop_front();

                resolver.signal.store( true, std::memory_order_release );
                resolver.signal.notify_one();
            break; }

            case BAR_PROTO_OP_NAK: {
                if( _resolvers.empty() ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), but not resolver.";
                    return -1;
                }

                auto& resolver = _resolvers.front();

                if( head->_dw1.seq != resolver.seq ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), but resolver expects sequence ( " << resolver.seq << " ).";
                    return -1;
                }

                this->_atomic_resolvers_pop_front();

                resolver.signal.store( true, std::memory_order_release );
                resolver.signal.notify_one();

                return -1;
            break; }

            case BAR_PROTO_OP_BURST: {
                return this->BTH_SOCKET::itr_recv( ( char* )direct, head->_dw2.sz, echo ) > 0 ? 0 : -1;
            }

            default: {
                echo( this, ECHO_LEVEL_ERROR ) << "Unknown operation code ( " << ( int )head->_dw0.op << " ).";
                return -1;
            }
        }

        return 0;
    }

public:
    DWORD ping( _ENGINE_COMMS_ECHO_RT_ARG ) {
        this->atomic_acquire_seq();
        head->_dw0.op = BAR_PROTO_OP_PING;
        head->_dw2.sz = 0;

        echo( this, ECHO_LEVEL_PENDING ) << "Pinging on sequence ( " << head->_dw1.seq << " )... ";

        this->_emplace_resolver_and_out_cache_write( nullptr, 0, echo )->wait( false );
        
        echo( this, ECHO_LEVEL_OK ) << "Received ping acknowledgement.";
        return 0;
    }

    DWORD get( std::string_view str_id, void* dest, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        this->atomic_acquire_seq();
        head->_dw0.op = BAR_PROTO_OP_GET;
        head->_dw2.sz = str_id.length() + 1;

        strcpy( ( char* )data, str_id.data() );

        this->_emplace_resolver_and_out_cache_write( dest, sz, echo )->wait( false );

        return 0;
    }

    DWORD set( std::string_view str_id, void* src, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        this->atomic_acquire_seq();
        head->_dw0.op = BAR_PROTO_OP_SET;
        head->_dw2.sz = str_id.length() + 1 + sz;

        strcpy( ( char* )data, str_id.data() );
        memcpy( ( char* )data + str_id.length() + 1, src, sz );

        this->_emplace_resolver_and_out_cache_write( nullptr, 0, echo )->wait( false );

        return 0;
    }

public: 
    DWORD listen_trust( bar_ctrl::dynamic_state_t* dy_st, _ENGINE_COMMS_ECHO_RT_ARG ) {
    l_listen_begin: {
        auto head = this->_listen_head( echo );
        
        if( DWORD result = this->_resolve_head( &head, ( void* )dy_st, echo ); result != 0 ) return result;
        if( head._dw0.op != BAR_PROTO_OP_BURST ) goto l_listen_begin;
    }
        return 0;
    }

};



}; };
