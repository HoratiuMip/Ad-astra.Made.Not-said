#include <IXT/init.hpp>
#include <IXT/comms.hpp>
#include <IXT/SpecMod/barracuda_controller.hpp>

using namespace IXT;

int main() {
    SpecMod::BarracudaController bc;

    if( auto status = IXT::initial_uplink( 0, nullptr, INIT_FLAG_UPLINK_NETWORK, nullptr, nullptr ); status != 0 ) return status;

    if( auto status = bc.data_link( L"BARRACUDA", 0 ); status != 0 ) return status; 

    bc.write( "BARRACUDA", strlen( "BARRACUDA" ) );
    
    char buffer[ 513 ]; buffer[ 512 ] = 0;

    while( true ) {
        IXT::DWORD count = bc.read( buffer, 512 );

        if( count < 0 ) {
            comms( &bc, ECHO_LEVEL_ERROR ) << "Read fault, breaking from main loop.";
            break;
        }

        buffer[ count ] = 0;

        comms( &bc, ECHO_LEVEL_INTEL ) << "RX'd:\n" << buffer;
    }
    
    return 0;
}
