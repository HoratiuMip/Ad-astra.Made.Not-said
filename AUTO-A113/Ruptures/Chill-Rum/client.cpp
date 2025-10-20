#include <A113/OSp/OSp.hpp>
using namespace A113::BRp;
using namespace A113::OSp;
#include <nlohmann/json.hpp>

#include "common.hpp"

#include <thread>
#include <format>

namespace ChRum {


class Client : public IPv4_TCP_socket {
public:
    inline static constexpr int   MAX_CONN_ATTEMPTS   = 0x5;
    inline static constexpr int   CONN_RETRY_AFTER    = 0x2;

public:
    Client() = default;

protected:
    std::thread   _ht_speak    = {};
    std::thread   _ht_listen   = {};

protected:
    void _main_speak( void ) {

    }

    void _main_listen( void ) {
    while( true ) {
        HEAD head = {};

        int result = this->basic_read_loop( { ( char* )&head, sizeof( HEAD ) } );
        A113_ASSERT_OR( sizeof( HEAD ) == result ) {
            spdlog::error( "Head reception from {}:{}.", ( char* )_conn.addr_str, _conn.port );
            return;
        }

        A113_ASSERT_OR( 0x0 == strcmp( head._align, "ChRum" ) && ( head.sz > 0 && head.sz <= 1024 ) ) {
            spdlog::error( "Bad head from {}:{}.", ( char* )_conn.addr_str, _conn.port );
            return;
        }

        char payload[ head.sz + 1 ];
        result = this->basic_read_loop( { payload, head.sz } );
        A113_ASSERT_OR( head.sz == result ) {
            spdlog::error( "Payload reception from {}:{}.", ( char* )_conn.addr_str, _conn.port );
            return;
        }

        payload[ head.sz ] = 0x0; 
        spdlog::info( "Payload received: \"{}\"", ( char* )payload );
    } }

public:
    bool join( const char* addr, uint16_t port ) {
        for( int attempt = 0x1; attempt <= MAX_CONN_ATTEMPTS; ++attempt ) {
            A113_ASSERT_OR( 0x0 == this->uplink( addr, port ) ) {
                spdlog::warn( "Could not connect to {}:{}, retrying in {}s ({}/{})...", addr, port, CONN_RETRY_AFTER, attempt, MAX_CONN_ATTEMPTS );
                continue;
            }
            goto l_conn_ok;
        }
        spdlog::error( "Failed to join." );
        return false;

    l_conn_ok:
        _ht_speak = std::thread( &Client::_main_speak, this );
        _ht_listen = std::thread( &Client::_main_listen, this );

        return true;
    }

};


};

#include <iostream>
int main( int argc, char* argv[] )  {
    init( argc, argv, { flags: InitFlags_Sockets } );

    ChRum::Client client;
    client.join( "127.0.0.1", 80 );

    struct {    
        ChRum::HEAD head = {};
        char        data[ 1024 ];
    } payload;

    while( true ) {
        std::string str;
        std::getline( std::cin, str );

        str = std::format( "{{ message: \"{}\" }}", str.c_str() );

        strcpy( payload.data, str.c_str() );
        payload.head.sz = str.length();
        client.basic_write_loop( A113::BUFFER{ ( char* )&payload, sizeof( ChRum::HEAD ) + payload.head.sz } );
    }

    return 0x0;
}