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


class _PlaceHandlerImpl {

};

template< typename T > class PlaceHandler : public _PlaceHandlerImpl {

};


class _TransitionHandlerImpl {
public:
    virtual void TransitionDelay( void ) = 0;

    virtual bool TransitionGuardsMappings( void ) = 0;

    virtual void Init( std::string_view name, std::shared_ptr< _PlaceHandlerImpl > ph ) = 0;

    virtual void SetDelay( int value ) = 0;

    virtual void SetDelayInRange( int eet, int let ) = 0;

};




int main( int argc, char* argv[] ) {
    srand( time( nullptr ) );

    ixN::Fwk::Threads_follow fwk;
    FWK = &fwk;

    fwk.launch( "UserThread", 3, [ & ] ( void* ) -> void {
    while( true ) {
        
    }
    }, nullptr );
    
    fwk.launch( "BGThread", 3, [ & ] ( void* ) -> void {
    while( true ) {
        FWK->im_at( 0 );
        FWK->im_next();
        
        
    }
    }, nullptr );

    return fwk.main( argc, argv );
}