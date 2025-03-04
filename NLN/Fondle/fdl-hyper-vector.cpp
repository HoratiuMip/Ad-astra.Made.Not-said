/*
*/
#include <NLN/hyper-vector.hpp>
#include <NLN/comms.hpp>

using namespace NLN;



int main() {
    auto hvec = HVEC< int >::allocc( 0x9 );

    comms() << *hvec; 
}