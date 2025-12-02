//#include <A113/BRp/block_diffuser.hpp>
#include <A113/OSp/render3.hpp>

#include <thread>
#include <string>
#include <iostream>

struct X {
    X() = default;
    X( int x ) : x{ x } { std::cout << "Created " << x << '\n'; }
    ~X() { x = 0; std::cout << "Destroyed " << x << '\n'; }
    int x;
};

int main( int argc, char* argv[] ) {
    a113::render::Cluster cluster{ nullptr };
    cluster.pipe_cache().make_pipe_from_prefixed_path( "C:/Hackerman/Git/For_Academical_Purposes/IXN/Ruptures/warc/imm/earth/earth" );


    // Block_diffuser< X > bf;

    // Block_diffuser< X >::block_t ints[ 10 ];
    // for( auto& n : ints ) n.content.x = 0x0;

    // bf.bind_memory( &ints[ 0 ], 10 );
    // bf.inject( 5 );
    // bf.inject( 6 );
    // bf.inject( 7 );
    // bf.eject( ints + 1 );
    // bf.inject( 6 );
    // bf.inject( 8 );
    // bf.eject( ints );
    // bf.inject( 5 );
    // bf.inject( 9 );


    // for( auto& n : ints )
    //     std::cout << n.content.x << ' ';
    // std::cout << '\n';

    return 0;
}