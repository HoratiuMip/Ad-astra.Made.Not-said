/*
*/
#include <iostream>
#include <conio.h>

#include <IXT/audio.hpp>
#include <IXT/endec.hpp>

using namespace IXT;



int main() {
    Endec::Wav< int > sample_wav{ ASSET_WAV_SAX_PATH };

    auto audio = std::make_shared< Audio >( 
        Audio::devices()[ 0 ], 
        sample_wav.sample_rate, sample_wav.tunnel_count, 
        32, 256 
    );


    std::map< decltype( _getch() ), std::shared_ptr< Wave > > sample_map{
        { '1', std::make_shared< Sound >( audio, ASSET_WAV_SAX_PATH ) },
        { '2', std::make_shared< Sound >( audio, ASSET_WAV_GANGNAM_PATH ) },
        { '3', std::make_shared< Sound >( audio, ASSET_WAV_90S_PATH ) },
        { 'q', std::make_shared< Synth >( audio, Synth::gen_sine( 0.5, 220 ), 3.0 ) },
        { 'w', std::make_shared< Synth >( audio, Synth::gen_sine( 0.5, 440 ), 3.0 ) },
        { 'e', std::make_shared< Synth >( audio, Synth::gen_sine( 0.5, 880 ), 3.0 ) }
    };


    std::string_view menu_str = 
    "1, 2, 3 - play waves.\n"
    "q, w, e\n"
    "4, 5    - volume up/down.\n"
    "6, 7    - velocity up/down.\n"
    "8       - loop all.\n"
    "9       - stop all.\n"
    "0       - exit.\n";
    std::cout << menu_str;
    
    for( auto cmd = std::tolower( _getch() ); cmd != '0'; cmd = std::tolower( _getch() ) ) {
        try {
            audio->play( sample_map.at( cmd ) );
        } catch( std::out_of_range& err ) {
            switch( cmd ) {
                case '4': [[fallthrough]];
                case '5':
                    audio->volume_tweak( std::plus{}, cmd == '4' ? -0.1 : 0.1 );
                    break;

                case '6': [[fallthrough]];
                case '7':
                    audio->velocity_tweak( std::plus{}, cmd == '6' ? -0.1 : 0.1 );
                    break;

                case '8':
                    for( auto& w : sample_map )
                        w.second->loop_tweak();
                    break;

                case '9':
                    audio->stop();
                    break;
            }
        }

        std::cout << menu_str << '\n'
        << std::left << std::setw( 10 ) << "Volume: " << audio->volume() << '\n'
        << std::left << std::setw( 10 ) << "Velocity: " << audio->velocity() << '\n'
        << std::left << std::setw( 10 ) << "Looping: " << std::boolalpha << sample_map.begin()->second->is_looping() << "\n\n";
    }

}