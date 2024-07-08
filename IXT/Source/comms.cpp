/*
*/

#include <IXT/comms.hpp>

#include <IXT/descriptor.hpp>



namespace _ENGINE_NAMESPACE {



Echo::Echo()
: _dump{ comms.new_echo_dump() }
{}

Echo::~Echo() {
    if( _depth > 0 ) return;
    
    if( _dump == nullptr ) return;

    this->_str().put( 0 );
    comms.out( *this );

    if( _depth == 0 )
        comms.delete_echo_dump( std::exchange( _dump, nullptr ) );
}



void Comms::_flush( OS::sig_t code ) {
    for( auto dump : comms._supervisor )
        Echo{ dump, -1 };
}



};
