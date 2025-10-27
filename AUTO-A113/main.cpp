#include <A113/OSp/hyper_net.hpp>
using namespace A113::OSp;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <thread>
#include <string>
#include <iostream>

int main( int argc, char* argv[] ) {
    A113::OSp::init( argc, argv, { flags: InitFlags_None } );

    int flag = 0x0;

    HyperNet_Executor hnex{ "Net0", true };
    hnex.push_sink( new HyperNet_Sink{ { name: "p0" } } );
    hnex.push_sink( new HyperNet_Sink{ { name: "p1" } } );
    hnex.push_drain( new HyperNet_Drain_Lambda{ { name: "t0" }, [ & ] ( HyperNet_Token* tok_ ) -> A113::RESULT {
        return flag & 1;
    } } );
    hnex.push_drain( new HyperNet_Drain_Lambda{ { name: "t1" }, [ & ] ( HyperNet_Token* tok_ ) -> A113::RESULT {
        return ( flag & 1 ) == 0;
    } } );

    hnex.bind_SDS( "p0", "t0", "p1" );
    hnex.bind_SDS( "p1", "t1", "p0" );
    hnex.insert( "p0", new HyperNet_Token{} );

    auto th = std::thread{ [ & ] ( void ) -> void {
    for(;;) {
        hnex.clock( 0.0 );
    } } };

    for(;;) {
        std::this_thread::sleep_for( std::chrono::seconds{ 2 } ); ++flag;
    }

    return 0;
}