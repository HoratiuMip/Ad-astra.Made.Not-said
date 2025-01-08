/*
*/
#include <IXT/hyper-vector.hpp>
#include <IXT/comms.hpp>

using namespace IXT;



int main() {
    auto hvec = HVEC< int >::allocc( 0x9 );

    comms() << *hvec; 
}