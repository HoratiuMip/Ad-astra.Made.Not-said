#include <IXN/Framework/threads_follow.hpp>
#include <random>


static ixN::Fwk::Threads_follow*   FWK;


void sleep( int ms_min, int ms_max ) { 
    static std::random_device rd;
    int ms = rd() % ( ms_max - ms_min ) + ms_min;
    std::this_thread::sleep_for( std::chrono::milliseconds( ms ) );
}


void thread_1( void* arg ) {
    while( true ) {
        FWK->im_at( 0 );
        sleep( 100, 1000 );
        for( int n = 1; n < ( int64_t )arg; ++n ) {
            FWK->im_next();
            sleep( 100, 1000 );
        }
    }
}

void thread_2( void* arg ) {
    FWK->im_at( 0 );
    sleep( 100, 1000 );
    for( int n = 1; n < ( int64_t )arg; ++n ) {
        FWK->im_next();
        sleep( 100, 1000 );
    }
}


int main( int argc, char* argv[] ) {
    srand( time( nullptr ) );

    ixN::Fwk::Threads_follow fwk;
    FWK = &fwk;

    fwk.launch( "Thread1", 50, thread_1, (void*)50 );
    fwk.launch( "Thread2", 40, thread_2, (void*)40 );
    fwk.launch( "Thread3", 30, thread_1, (void*)30 );
    fwk.launch( "Thread4", 50, thread_1, (void*)50 );
    fwk.launch( "Thread5", 40, thread_2, (void*)40 );

    return fwk.main( argc, argv );
}