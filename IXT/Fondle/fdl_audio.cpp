/*
*/
#include <iostream>
#include <conio.h>

#include <IXT/audio.hpp>
#include <IXT/endec.hpp>

using namespace IXT;



int main() {
    Endec::Wav< int > sample_wav{ ASSET_WAV_SAX_PATH };

    Audio audio{ 
        Audio::devices()[ 0 ], 
        sample_wav.sample_rate, sample_wav.channel_count, 
        32, 256 
    };

    std::map< decltype( _getch() ), std::shared_ptr< Wave > > sample_map{
        { '1', std::make_shared< Sound >( audio, ASSET_WAV_SAX_PATH ) },
        { '2', std::make_shared< Sound >( audio, ASSET_WAV_GANGNAM_PATH ) },
        { '3', std::make_shared< Sound >( audio, ASSET_WAV_90S_PATH ) }
    };

    
    for( auto cmd = _getch(); cmd != '0'; cmd = _getch() ) {
        try {
            sample_map.at( cmd )->play();
        } catch( std::out_of_range& err ) {
            std::cout << err.what() << '\n';
        }
    }

}