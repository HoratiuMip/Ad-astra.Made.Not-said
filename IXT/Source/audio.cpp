/*
*/

#include <IXT/audio.hpp>

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



bool Wave::is_playing() const {
    return _audio->is_playing( *this );
}

void Wave::play() {
    _audio->play( *this );
}

void Wave::play( HVEC< Wave > self ) {
    _audio->play( std::move( self ) );
}



};