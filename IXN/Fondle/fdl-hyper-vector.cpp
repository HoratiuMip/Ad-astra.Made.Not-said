/*
*/
#include <IXN/hyper-vector.hpp>
#include <IXN/comms.hpp>

using namespace ixN;



int main() {
    auto HVec = HVec< int >::allocc( 0x9 );

    comms() << *HVec; 
}