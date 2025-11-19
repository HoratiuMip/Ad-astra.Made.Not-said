#include <A113/OSp/OSp.hpp>
#include <A113/BRp/wjpv3_utils.hpp>
using namespace a113;
#include <nlohmann/json.hpp>

#include <thread>
#include <format>
#include <iostream>

namespace ChRum {


class Client : public wjpv3::LMHIPayload_InternalBufs_on< io::IPv4_TCP_socket, 1024, 1024 > {
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
    for(;;) {
        std::string str;
        std::getline( std::cin, str );

        WJPInfo_TX info;
        this->lmhi_tx_payload( 
            &info, 
            snprintf( this->payload().ptr, this->payload().n, "{ message: \"%s\" }", str.c_str() ) 
        );
    } }

    void _main_listen( void ) {
    for(;;) {
        WJPInfo_RX info;
        this->WJP_RX_lmhi( &info );
        spdlog::critical( "{}", WJP_err_strs[ info.err ] );
    } }

    virtual int WJP_lmhi_when_recv( WJP_LMHIReceiver::Layout* lo ) override {
        spdlog::info( "Recieved broadcast: \"{}\".", std::string{ lo->payload_in.addr, lo->payload_in.sz }.c_str() );
        return 0x0;
    }

public:
    bool join( const char* addr, uint16_t port ) {
        this->bind_peer( addr, port );
        for( int attempt = 0x1; attempt <= MAX_CONN_ATTEMPTS; ++attempt ) {
            A113_ASSERT_OR( 0x0 == this->uplink() ) {
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


int main( int argc, char* argv[] )  {
    init( argc, argv, { flags: InitFlags_Sockets } );

    ChRum::Client client;
    client.join( "127.0.0.1", 80 );

    std::this_thread::sleep_for( std::chrono::hours{ 1 } );
    return 0x0;
}