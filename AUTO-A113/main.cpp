//#include <A113/BRp/block_diffuser.hpp>
#include <a113/fwork/immersive.hpp>

#include <thread>
#include <string>
#include <iostream>


int main( int argc, char* argv[] ) {
    a113::fwork::Immersive imm;
    imm.main( argc, argv, a113::fwork::Immersive::config_t{
        
    } );

    return 0;
}