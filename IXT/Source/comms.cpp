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

    this->_any_str().put( 0 );
    comms.out( *this );

    if( _depth == 0 )
        comms.delete_echo_dump( std::exchange( _dump, nullptr ) );
}

Echo::out_stream_t& Echo::_any_str() {
    return _dump != nullptr ? this->_acc_str() : *comms._stream;
}

Echo& Echo::push_desc( Echo::descriptor_t desc ) {
    if( _dump != nullptr ) {
        this->_descs().emplace_back( desc );
        this->_acc_str() << desc_switch;
    } else {
        std::invoke( comms._desc_proc, desc );
    }

    return *this;
}



void Comms::_flush( OS::sig_t code ) {
    for( auto dump : comms._supervisor )
        Echo{ dump, -1 };
}



};
