#pragma once
/*===== NLN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Driver for the BarraCUDA controller.
|
======*/

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>
#include <NLN/network.hpp>


#define BARRACUDA_CTRL_BUILD_FOR_ENGINE_DRIVER
#define BARRACUDA_CTRL_ARCHITECTURE_LITTLE
#include "../../../../Devices/BarraCUDA-CTRL/barracuda-ctrl.hpp"



namespace _ENGINE_NAMESPACE { namespace _ENGINE_DEVICE_NAMESPACE {



enum BARRACUDA_CTRL_FLAG : DWORD {
    BARRACUDA_CTRL_FLAG_NO_BLOCK = 1 << 0,
    BARRACUDA_CTRL_FLAG_TRUST    = 1 << 1,

    _BARRACUDA_CTRL_FLAG_FORCE_DWORD = 0x7F'FF'FF'FF
};

class BarracudaController : public BTH_SOCKET, public bar_ctrl::out_cache_t< 128 > {
public:
    _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "BarracudaController" );

public:
    BarracudaController() = default;

    ~BarracudaController() {
        if( this->connected() ) closesocket( std::exchange( _bt_socket, ::SOCKET{} ) );
    }

public:
    struct resolver_t {
        int32_t            seq;
        void*              dest;
        int16_t            sz;
        std::atomic_bool   signal;
    };

_ENGINE_PROTECTED:
    BTH_ADDR                   _on_board_uc_bt_addr   = {};
    ::SOCKET                   _bt_socket             = {};

    std::mutex                 _write_mtx             = {};

    std::thread                _resolver              = {};
    bool                       _trust_ex              = false;
    std::deque< resolver_t >   _resolvers             = {};
    std::mutex                 _resolvers_mtx         = {}; 

public:
    bool connected() {
        return _bt_socket != ::SOCKET{};
    }

public:
    DWORD data_link( DWORD flags, _ENGINE_COMMS_ECHO_RT_ARG ) {
        _trust_ex = flags & BARRACUDA_CTRL_FLAG_TRUST;

        if( DWORD ret = this->BTH_SOCKET::connect( bar_ctrl::DEVICE_NAME_W ); ret != 0 ) return ret;
        
        if( !_trust_ex ) {
            if( DWORD rez = this->ping( echo ); rez != 0 ) return rez;
        }

        return 0;
    }

_ENGINE_PROTECTED:
    DWORD _read( char* buffer, DWORD count, _ENGINE_COMMS_ECHO_RT_ARG ) {
        DWORD crt_count = 0;
        
        do {
            DWORD result = recv( BTH_SOCKET::_socket, buffer + crt_count, count - crt_count, MSG_WAITALL );
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
            DWORD result = send( BTH_SOCKET::_socket, buffer + crt_count, count - crt_count, 0 );
             if( result <= 0 ) { 
                echo( this, ECHO_LEVEL_ERROR ) << "TX fault( " << result << " | " << WSAGetLastError() << " )."; 
                return result; 
            }
            crt_count += result;
        } while( crt_count < count );

        if( crt_count < count ) { 
            echo( this, ECHO_LEVEL_ERROR ) << "TX fault, too many bytes written."; 
            return -1; 
        }

        return count;
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
    int _out_cache_write( void ) override {
        return this->_write( ( char* )_out_cache, sizeof( *_out_cache_head ) + _out_cache_head->_dw2.sz );
    }

    std::atomic_bool* _emplace_resolver_and_out_cache_write( void* dest, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG  ) {
        std::unique_lock lock{ _resolvers_mtx };

        auto& resolver = this->_emplace_resolver( _out_cache_head->_dw1.seq, dest, sz );
        
        if( DWORD rez = this->_out_cache_write(); rez <= 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Cache write fault on sequence ( " << _out_cache_head->_dw1.seq << " ).";
            this->_resolvers.pop_back();
            return nullptr;
        }

        return &resolver.signal;
    }

    bar_ctrl::proto_head_t _listen_head( _ENGINE_COMMS_ECHO_RT_ARG ) {
        bar_ctrl::proto_head_t head;
        if( this->_read( ( char* )&head, sizeof( head ) ) <= 0 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Head read fault.";
            return bar_ctrl::proto_head_t{};
        } 
        return head;
    }

    DWORD _resolve_head( bar_ctrl::proto_head_t* head, void* direct, _ENGINE_COMMS_ECHO_RT_ARG ) {
        if( !head->is_signed() ) { 
            echo( this, ECHO_LEVEL_ERROR ) << "Bad head signature ( " << ( head->sig & bar_ctrl::PROTO_SIG_MSK ) << ")."; 
            return -1; 
        }
    
        switch( head->_dw0.op ) {
            case bar_ctrl::PROTO_OP_NULL: {
                echo( this, ECHO_LEVEL_ERROR ) << "Cannot resolve head with NULL operation code.";
                return -1;
            }

            case bar_ctrl::PROTO_OP_ACK: {
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

                if( this->_read( ( char* )resolver.dest, resolver.sz, echo ) <= 0 ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Inbound ACK on sequence ( " << head->_dw1.seq << " ), fault on data read.";
                    return -1;
                }

            l_signal:
                this->_atomic_resolvers_pop_front();

                resolver.signal.store( true, std::memory_order_release );
                resolver.signal.notify_one();
            break; }

            case bar_ctrl::PROTO_OP_NAK: {
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

            case bar_ctrl::PROTO_OP_DYNAMIC: {
                return this->_read( ( char* )direct, head->_dw2.sz, echo ) > 0 ? 0 : -1;
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
        _out_cache_head->atomic_acquire_seq();
        _out_cache_head->_dw0.op = bar_ctrl::PROTO_OP_PING;
        _out_cache_head->_dw2.sz = 0;

        echo( this, ECHO_LEVEL_PENDING ) << "Pinging on sequence ( " << _out_cache_head->_dw1.seq << " )... ";

        this->_emplace_resolver_and_out_cache_write( nullptr, 0, echo )->wait( false );
        
        echo( this, ECHO_LEVEL_OK ) << "Received ping acknowledgement.";
        return 0;
    }

    DWORD get( std::string_view str_id, void* dest, int16_t sz, _ENGINE_COMMS_ECHO_RT_ARG ) {
        _out_cache_head->atomic_acquire_seq();
        _out_cache_head->_dw0.op = bar_ctrl::PROTO_OP_GET;
        _out_cache_head->_dw2.sz = str_id.length() + 1;

        strcpy( ( char* )_out_cache_data, str_id.data() );

        this->_emplace_resolver_and_out_cache_write( dest, sz, echo )->wait( false );

        return 0;
    }

public: 
    DWORD listen_trust( bar_ctrl::dynamic_state_t* dy_st, _ENGINE_COMMS_ECHO_RT_ARG ) {
    l_listen_begin: {
        auto head = this->_listen_head( echo );
        
        if( DWORD result = this->_resolve_head( &head, ( void* )dy_st, echo ); result != 0 ) return result;
        if( head._dw0.op != bar_ctrl::PROTO_OP_DYNAMIC ) goto l_listen_begin;
    }
        return 0;
    }

};



}; };
