#include <A113/OSp/OSp.hpp>
#include <A113/BRp/wjpv3_utils.hpp>
using namespace a113;
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <thread>
#include <list>
#include <queue>

namespace ChRum {


class Server {
protected:
    friend class Client;
    class Client : public wjpv3::LMHIPayload_InternalBufs_on< io::IPv4_TCP_socket, 0, 1024 > {
    public:
        Client() = default;

    protected:
        std::thread   _ht_main   = {};
        Server*       _srv       = nullptr;

    protected:
        void _main( void ) {
        for(;;) {
            WJPInfo_RX info;
            this->WJP_RX_lmhi( &info );
        } }

        virtual int WJP_lmhi_when_recv( WJP_LMHIReceiver::Layout* lo ) override {
            std::string json_str{ lo->payload_in.addr, lo->payload_in.sz };
            spdlog::info( "Received payload from {}:{}, on action [{}], \"{}\".", this->addr_c_str(), this->port(), lo->head_in._dw1.ACT, json_str.c_str() );

            _srv->push( json_str );
            return 0x0;
        }

    public:
        void start( Server* srv ) {
            _srv = srv;
            this->bind_TX_buffer( _srv->_bcasts_tx_buf );
            _ht_main = std::thread( _main, this );
        }   

    };

public:
    Server() {
        *( WJP_Head* )&_bcasts_buf_1 = WJP_Head{};
    }

protected:
    uint16_t                      _port           = 0x0;

    std::thread                   _ht_main_conn   = {};
    std::thread                   _ht_main_cast   = {};

    std::list< Server::Client >   _clients        = {};
    std::mutex                    _clients_mtx    = {};

    std::queue< std::string >     _bcasts         = {};
    std::atomic_int               _bcasts_size    = { 0x0 };
    std::mutex                    _bcasts_mtx     = {};
    char                          _bcasts_buf_1[ 1024 ];
    MDsc                          _bcasts_tx_buf  ={ _bcasts_buf_1, sizeof( _bcasts_buf_1 ) };

protected:
    void _main_conn( void ) {
    for(;;) {
            std::unique_lock lock{ _clients_mtx };
            _clients.emplace_front();
            auto client = _clients.begin();
            lock.unlock();

            client->bind_peer( a113::io::ipv4_addr_t{ 0x0 }, _port );
            A113_ASSERT_OR( 0x0 == client->listen() ) {
                lock.lock();
                _clients.erase( client );
                continue;
            }

            lock.lock();
            client->start( this );
    } }

    void _main_cast( void ) {
    for(;;) {
        _bcasts_size.wait( 0x0, std::memory_order_seq_cst );
        _bcasts_size.fetch_sub( 1, std::memory_order_seq_cst );

        std::unique_lock lock{ _bcasts_mtx };
        std::string json = std::move( _bcasts.front() );
        _bcasts.pop();
        lock.unlock();

        int bcast_sz = snprintf( _bcasts_tx_buf.ptr + sizeof( WJP_Head ), _bcasts_tx_buf.n - sizeof( WJP_Head ), "%s", json.c_str() );

        std::unique_lock lock2{ _clients_mtx };
        for( auto& client : _clients ) {
            if( &client == &_clients.front() ) continue;
            WJPInfo_TX info;
            spdlog::info( "Broadcasting payload to {}:{}...", client.addr_c_str(), client.port() );
            if( client.lmhi_tx_payload( &info, bcast_sz ) <= 0 ) {
                spdlog::warn( "Failed to broadcast the payload to {}:{}, [{}].", client.addr_c_str(), client.port(), WJP_err_strs[ info.err ] );
            }
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