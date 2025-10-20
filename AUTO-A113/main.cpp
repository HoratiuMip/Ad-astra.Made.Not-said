#include <A113/OSp/IO_sockets.hpp>
using namespace A113::OSp;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <thread>
#include <string>
#include <iostream>

int main( int argc, char* argv[] ) {
    Internal.init( argc, argv, { flags: INTERNAL::InitFlag_Sockets } );

    auto th1 = std::thread{ [] () -> void {
        IPv4_TCP_socket socket;
        socket.listen( 80 );

        char msg[ 5 ]; msg[ 4 ] = 0;
        A113::BUFFER buf{ msg, 4 };
        while( 1 ) {
            socket.basic_read_loop( buf );
            spdlog::info( "{}", msg );
        }
    } };

    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );

    auto th2 = std::thread{ [] () -> void {
        IPv4_TCP_socket socket;
        socket.connect( "127.0.0.1", 80 );
        while( 1 ) {
            std::string s;
            std::cin>>s;
            
            socket.basic_write_loop( { ptr: s.data(), n: 4 } );
        }
    } };

    th1.join(); th2.join();

    return 0;
}