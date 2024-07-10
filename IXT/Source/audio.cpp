/*
*/

#include <IXT/audio.hpp>

#include <IXT/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



bool Wave::is_playing() const {
    return std::find( _audio->_waves.begin(), _audio->_waves.end(), this )
           !=
           _audio->_waves.end();
}

void Wave::play() {
    this->prepare_play();

    if( !this->is_playing() )
        _audio->_waves.push_back( this );
}



};