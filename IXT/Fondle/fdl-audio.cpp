/*
*/
#include <iostream>
#include <conio.h>

#include <IXT/audio.hpp>
#include <IXT/endec.hpp>
#include <IXT/hyper-vector.hpp>

using namespace IXT;



int main() { 
    Endec::Wav< int > sample_wav{ ASSET_WAV_SAX_PATH };
    
    auto audio = HVEC< Audio >::alloc( 1,
        Audio::devices()[ 0 ], 
        sample_wav.sample_rate, sample_wav.tunnel_count, 
        32, 256
    );
    

    std::map< decltype( _getch() ), HVEC< Wave > > sample_map{
        { '1', HVEC< Sound >::alloc( 1, audio, ASSET_WAV_SAX_PATH ) },
        { '2', HVEC< Sound >::alloc( 1, audio, ASSET_WAV_GANGNAM_PATH ) },
        { '3', HVEC< Sound >::alloc( 1, audio, ASSET_WAV_90S_PATH ) },
        { 't', HVEC< Sound >::alloc( 1, audio, ASSET_WAV_NOAA_PATH ) },
        { 'q', HVEC< Synth >::alloc( 1, audio, Synth::gen_sine( 0.5, 220 ), 3.0 ) },
        { 'w', HVEC< Synth >::alloc( 1, audio, Synth::gen_sine( 0.5, 440 ), 3.0 ) },
        { 'e', HVEC< Synth >::alloc( 1, audio, Synth::gen_sine( 0.5, 880 ), 3.0 ) },
        { 'r', HVEC< Synth >::alloc( 1, audio, Synth::gen_cos( 0.5, 440 ), 3.0 ) }
    };


    std::string_view menu_str = 
    "[ 1, 2, 3, t ] - play waves.\n"
    "[ q, w, e, r ] /\n"
    "[ 4, 5 ]       - volume down/up.\n"
    "[ 6, 7 ]       - velocity down/up.\n"
    "[ m ]          - mute all.\n"
    "[ 8 ]          - loop all.\n"
    "[ 9 ]          - stop all.\n"
    "[ 0 ]          - exit.\n";

    auto crs = OS::console.crs();
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

                case 'm':
                    audio->mute_tweak();
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

        OS::console.crs_at( crs );
        std::cout << menu_str << '\n'
        << std::left << std::setw( 10 ) << "Volume: " << std::setw( 16 ) << audio->volume() << '\n'
        << std::left << std::setw( 10 ) << "Velocity: " << std::setw( 16 ) << audio->velocity() << '\n'
        << std::left << std::setw( 10 ) << "Looping: " << std::boolalpha << sample_map.begin()->second->is_looping() << " \n\n";

        
        static char progress_chars[] = { '|', '/', '-', '\\' };
        static uint64_t progress_at = 0;

        std::cout << "[ " << progress_chars[ progress_at++ % std::size( progress_chars ) ] << " ]";
    }

}