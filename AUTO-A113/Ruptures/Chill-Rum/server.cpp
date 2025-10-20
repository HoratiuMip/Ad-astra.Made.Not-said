#include <A113/OSp/OSp.hpp>
using namespace A113::BRp;
using namespace A113::OSp;
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "common.hpp"

#include <thread>
#include <list>
#include <queue>

namespace ChRum {


class Server {
protected:
    class Client : public IPv4_TCP_socket {
    public:
        Client() = default;

    protected:
        std::thread   _ht_main   = {};
        Server*       _srv       = nullptr;

    protected:
        void _main( void ) {
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

            _srv->push( payload );
        } }

    public:
        void start( Server* srv ) {
            _srv = srv;
            _ht_main = std::thread( _main, this );
        }   

    };

public:
    Server() = default;

protected:
    uint16_t                      _port           = 0x0;

    std::thread                   _ht_main_conn   = {};
    std::thread                   _ht_main_cast   = {};

    std::list< Server::Client >   _clients        = {};
    std::mutex                    _clients_mtx    = {};

    std::queue< std::string >     _bcasts         = {};
    std::atomic_int               _bcasts_size    = { 0x0 };
    std::mutex                    _bcasts_mtx     = {};

protected:
    void _main_conn( void ) {
    while( true ) {
            std::unique_lock lock{ _clients_mtx };
            _clients.emplace_front();
            auto client = _clients.begin();
            lock.unlock();

            A113_ASSERT_OR( 0x0 == client->listen( _port ) ) {
                lock.lock();
                _clients.erase( client );
                continue;
            }

            lock.lock();
            client->start( this );
    } }

    void _main_cast( void ) {
    while( true ) {
        _bcasts_size.wait( 0x0, std::memory_order_seq_cst );
        _bcasts_size.fetch_sub( 1, std::memory_order_seq_cst );

        struct {
            HEAD   head   = {};
            char   data[ 1024 ];
        } payload;

        std::unique_lock lock{ _bcasts_mtx };
        std::string json = std::move( _bcasts.front() );
        _bcasts.pop();
        lock.unlock();

        payload.head.sz = json.length();
        strcpy( payload.data, json.c_str() );

        std::unique_lock lock2{ _clients_mtx };
        int total_sz = sizeof( HEAD ) + payload.head.sz;
        for( auto& client : _clients ) {
            if( &client == &_clients.front() ) continue;
            A113_ASSERT_OR( client.basic_write_loop( { ( char* )&payload, total_sz } ) == total_sz )
                spdlog::error( "Failed to send to {}:{}.", 0, 0 );
            else
                spdlog::info( "Send to {}:{}.", 0, 0 );
        }
        lock2.unlock();
    } }

public:
    bool start( uint16_t port ) {
        _port = port;
        _ht_main_conn = std::thread( &Server::_main_conn, this ); 
        _ht_main_cast = std::thread( &Server::_main_cast, this );

        return true;
    }

public:
    void push( std::string json ) {
        std::unique_lock lock{ _bcasts_mtx };
        _bcasts.emplace( std::move( json ) );
        lock.unlock();

        _bcasts_size.fetch_add( 1, std::memory_order_seq_cst );
        _bcasts_size.notify_one();
    }

};


};

int main( int argc, char* argv[] )  {
    init( argc, argv, { flags: InitFlags_Sockets } ); 

    ChRum::Server server;
    server.start( 80 );
    std::this_thread::sleep_for( std::chrono::hours( 1 ) );
    return 0x0;
}