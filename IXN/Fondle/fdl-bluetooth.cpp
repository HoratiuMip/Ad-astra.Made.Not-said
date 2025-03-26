#include <IXN/init.hpp>
#include <IXN/comms.hpp>
#include <IXN/SpecMod/barracuda-ctrl.hpp>

using namespace ixN;

int main() {
    SpecMod::BarracudaController bc;

    if( auto status = ixN::initial_uplink( 0, nullptr, INIT_FLAG_UPLINK_NETWORK, nullptr, nullptr ); status != 0 ) return status;

    if( auto status = bc.data_link( L"BARRACUDA", 0 ); status != 0 ) return status; 

    bc.write( "BARRACUDA", strlen( "BARRACUDA" ) );
    
    char buffer[ 513 ]; buffer[ 512 ] = 0;

    while( true ) {
        ixN::DWORD count = bc.read( buffer, 512 );

        if( count < 0 ) {
            comms( &bc, EchoLevel_Error ) << "Read fault, breaking from main loop.";
            break;
        }

        buffer[ count ] = 0;

        comms( &bc, EchoLevel_Info ) << "RX'd:\n" << buffer;
    }
    
    return 0;
}
