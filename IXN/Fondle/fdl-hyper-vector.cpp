/*
*/
#include <IXN/hyper-vector.hpp>
#include <IXN/comms.hpp>

using namespace ixN;



int main() {
    auto hvec = HVEC< int >::allocc( 0x9 );

    comms() << *hvec; 
}