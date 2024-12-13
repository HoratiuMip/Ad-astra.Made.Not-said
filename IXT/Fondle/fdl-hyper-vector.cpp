/*
*/
#include <IXT/hyper-vector.hpp>
#include <IXT/comms.hpp>

using namespace IXT;



int main() {
    auto hvec = HVEC< int >::alloc( 1, 0x9 );

    comms() << *hvec; 
}