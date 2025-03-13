/*
*/

#include <NLN/comms.hpp>

#include <NLN/descriptor.hpp>



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

_RtEchoOneLiner Echo::operator () ( const Descriptor& invoker, ECHO_LEVEL status ) {
    this->operator<<( '\n' );

    this->white()
    .operator<<( "[ " )
    .push_color( _status_colors[ status ] )
    .operator<<( _status_strs[ status ] )
    .white()
    .operator<<( " ]   \t" );

    this->white()
    .operator<<( "[ " )
    .gray()
    .operator<<( time( nullptr ) )
    .white()
    .operator<<( " ]" );
    
    this->blue();
    for( int64_t n = 1; n <= _depth; ++n )
        this->operator<<( '|' );
    
    const char* struct_name = invoker.struct_name();

    this->white()
    .operator<<( "[ " )
    .gray()
    .operator<<( struct_name ? struct_name : "NULL" )
    .white()
    .operator<<( " ][ " )
    .gray()
    .operator<<( invoker.xtdx() )
    .white()
    .operator<<( " ]" )
    .blue()
    .operator<<( " -> " )
    .white();

    return { comms, *this, nullptr, false, status };
}


void Comms::_flush( OS::sig_t code ) {
    for( auto dump : comms._supervisor )
        Echo{ dump, -1 };
}


_RtEchoOneLiner::_RtEchoOneLiner( Comms& c, Echo& e, const Descriptor* that, bool lock, ECHO_LEVEL level )
: comms{ c }, echo{ e }, locked{ lock } { if( locked ) comms._out_mtx.lock(); if( that != nullptr ) echo( that, level ); }

_RtEchoOneLiner::~_RtEchoOneLiner() { if( locked ) comms._out_mtx.unlock(); }



};
