#include <IXN/Framework/threads_follow.hpp>
#include <random>
#include <barrier>

#define TIME_UNIT_MS 500

static ixN::Fwk::Threads_follow*   FWK;


void sleep( int tu_min, int tu_max ) { 
    static std::random_device rd;
    int ms = rd() % ( tu_max*TIME_UNIT_MS - tu_min*TIME_UNIT_MS ) + tu_min*TIME_UNIT_MS;
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

int main( int argc, char* argv[] ) {
    srand( time( nullptr ) );


    std::barrier     T6{ 3 };
    std::mutex       P9;
    std::atomic_bool P10{ false };

    std::atomic_int  cycle_indicator{ 3 };

    /* V - Used only to sync the printing of barrier release. Does not relate to the exercices' Petri Net. - V */
    const auto indicate_cycle_end = [ & ] ( void ) -> void {
        if( cycle_indicator.fetch_sub( 1, std::memory_order_relaxed ) != 1 ) return;

        cycle_indicator = 3;
        ixN::comms( ixN::EchoLevel_Ok ) << "Barrier released.";
    };
    /* ^ - Used only to sync the printing of barrier release. Does not relate to the exercices' Petri Net. - ^ */

    ixN::Fwk::Threads_follow fwk;
    FWK = &fwk;

    fwk.launch( "Thread1", 3, [ & ] ( void* ) -> void {
    while( true ) {
        FWK->im_at( 0 ); ixN::comms( ixN::EchoLevel_Info ) << "th1 - P1";

        std::unique_lock lock{ P9 };

        FWK->im_at( 1 ); ixN::comms( ixN::EchoLevel_Info ) << "th1 - P4"; 
        sleep( 3, 7 );

        lock.unlock();

        FWK->im_at( 2 ); ixN::comms( ixN::EchoLevel_Info ) << "th1 - P7"; 

        indicate_cycle_end();
        T6.arrive_and_wait();
    }
    }, nullptr );
    
    fwk.launch( "Thread2", 3, [ & ] ( void* ) -> void {
    while( true ) {
        FWK->im_at( 0 ); ixN::comms( ixN::EchoLevel_Info ) << "th2 - P2";

        std::unique_lock lock{ P9 };

        FWK->im_at( 1 ); ixN::comms( ixN::EchoLevel_Info ) << "th2 - P5"; 
        sleep( 2, 4 );

        sleep( 2, 5 );
        lock.unlock();
        P10 = true; P10.notify_one();

        FWK->im_at( 2 ); ixN::comms( ixN::EchoLevel_Info ) << "th2 - P8"; 

        indicate_cycle_end();
        T6.arrive_and_wait();
    }
    }, nullptr );

    fwk.launch( "Thread3", 2, [ & ] ( void* ) -> void {
    while( true ) {
        FWK->im_at( 0 ); ixN::comms( ixN::EchoLevel_Info ) << "th3 - P3";

        P10.wait( false ); P10 = false;

        FWK->im_at( 1 ); ixN::comms( ixN::EchoLevel_Info ) << "th3 - P6"; 
        sleep( 4, 6 );

        indicate_cycle_end();
        T6.arrive_and_wait();
    }
    }, nullptr );

    return fwk.main( argc, argv );
}