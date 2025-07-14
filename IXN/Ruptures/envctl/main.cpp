#include <IXN/Framework/imgui_on_opengl3.hpp>
#include <IXN/audio.hpp>


#define ConfigMode_SFW  0x0
#define ConfigMode_NSFW 0x1
#define ConfigMode_OG   0x2
#define _ConfigMode_Upper 0x3
struct config_t {
    inline static const char*   INI_FILE_NAME   = "./envctl.ini";

    ixN::DWORD   mode     = ConfigMode_SFW;
    
    bool         lnch_m   = false;
    bool         lnch_d   = false;

    ixN::DWORD from_ini( void ) {
        std::ifstream file{ INI_FILE_NAME };

        if( !file ) {
            ixN::comms( ixN::EchoLevel_Warning ) << "Cannot open \"" << INI_FILE_NAME << "\".";
            return -1;
        }

        file.read( ( char* )this, sizeof( *this ) );

        if( int count = file.tellg(); count != sizeof( *this ) ) {
            ixN::comms( ixN::EchoLevel_Warning ) << "Did not read sizeof( *this ) == " << sizeof( *this ) << " bytes, read instead == " << count << " bytes.";
            return -1;
        }

        file.close();
        
        if( mode < 0 || mode >= _ConfigMode_Upper ) {
            ixN::comms( ixN::EchoLevel_Warning ) << " Invalid mode " << ( int )mode << ". Defaulting to SFW.";
            mode = ConfigMode_SFW;
        }

        return 0;
    }

    ixN::DWORD to_ini( void ) {
        std::ofstream file{ INI_FILE_NAME };

        file.write( ( char* )this, sizeof( *this ) );

        return 0;
    }
};


ixN::DWORD loop( double elapsed, void* arg ) {
    config_t& config = *( config_t* )arg;

    ImGui::SetNextWindowSize( ImVec2{ 256, 256 } );
    ImGui::Begin( "envctl" );

    ImGui::SeparatorText( "Mode" );
    if( ImGui::RadioButton( "SFW", config.mode == ConfigMode_SFW ) ) config.mode = ConfigMode_SFW;
    if( ImGui::RadioButton( "NSFW", config.mode == ConfigMode_NSFW ) ) config.mode = ConfigMode_NSFW;
    if( ImGui::RadioButton( "OG", config.mode == ConfigMode_OG ) ) config.mode = ConfigMode_OG;

    ImGui::SeparatorText( "Launch" );
    ImGui::Checkbox( "Spotify", &config.lnch_m );
    ImGui::SameLine();
    ImGui::Checkbox( "Discord", &config.lnch_d );

    ImGui::Separator();

    if( ImGui::Button( "Done" ) ) {
        ImGui::End();
        return -1;
    }

    ImGui::End();
    return 0;
} 


int execute( const config_t& config ) {
    std::string cmd = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe -Command \"envctl";

    cmd += ' ';

    switch( config.mode ) {
        case ConfigMode_SFW: cmd += "sfw"; break;
        case ConfigMode_NSFW: cmd += "nsfw"; break;
        case ConfigMode_OG: cmd += "og"; break;
    }

    cmd += ' ';

    if( config.lnch_m ) cmd += 'm';
    if( config.lnch_d ) cmd += 'd';

    cmd += '\"';

    return std::system( cmd.c_str() );
}


int main( int argc, char* argv[] ) {  
    config_t config;
    config.from_ini();

    ixN::Audio audio{ ixN::Audio::devices()[ 0 ], 44'100, 2 };
    ixN::Sound sound{ audio, "sound.wav" };

    ixN::Fwk::ImGui_on_OpenGL3 fwk;
    fwk.params.title      = "envctl";
    fwk.params.iconify    = true;
    fwk.params.is_running = true;
    fwk.params.arg        = &config;
    fwk.loop              = loop;
    fwk.init_hold         = false;
    
    if( ixN::DWORD status = fwk.main( argc, argv ); status != 0 ) {
        ixN::comms( ixN::EchoLevel_Error ) << "GUI error. Aborted.";
        return status;
    }

    sound.play();
    ixN::comms( ixN::EchoLevel_Pending ) << "Welcome aboard Captain.\n";

    int status = execute( config );
    config.to_ini();

    if( status == 0 ) {
        ixN::comms( ixN::EchoLevel_Ok ) << "All systems online.\n";
    } else {
        ixN::comms( ixN::EchoLevel_Warning ) << "Some systems may require your attention.\n";
    }

    std::this_thread::sleep_for( std::chrono::milliseconds{ ( int )( sound.duration() * 1000.0 ) } );
    return status;
}