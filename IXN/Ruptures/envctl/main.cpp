#include <IXN/Framework/imgui_on_opengl3.hpp>
#include <IXN/audio.hpp>


#define ConfigMode_SFW 0x0
#define ConfigMode_NSFW 0x1
struct config_t {
    ixN::DWORD   mode     = 0x0;
    
    bool         lnch_m   = false;
    bool         lnch_d   = false;
};


ixN::DWORD loop( double elapsed, void* arg ) {
    config_t& config = *( config_t* )arg;

    ImGui::SetNextWindowSize( ImVec2{ 256, 256 } );
    ImGui::Begin( "envctl" );

    ImGui::SeparatorText( "Mode" );
    if( ImGui::RadioButton( "SFW", config.mode == ConfigMode_SFW ) ) config.mode = ConfigMode_SFW;
    if( ImGui::RadioButton( "NSFW", config.mode == ConfigMode_NSFW ) ) config.mode = ConfigMode_NSFW;

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
    }

    cmd += ' ';

    if( config.lnch_m ) cmd += 'm';
    if( config.lnch_d ) cmd += 'd';

    cmd += '\"';

    return std::system( cmd.c_str() );
}


int main( int argc, char* argv[] ) {  
    config_t config;

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

    if( status == 0 ) {
        ixN::comms( ixN::EchoLevel_Ok ) << "All systems online.\n";
    } else {
        ixN::comms( ixN::EchoLevel_Warning ) << "Some systems require your attention.\n";
    }

    std::this_thread::sleep_for( std::chrono::milliseconds{ ( int )( sound.duration() * 1000.0 ) } );
    return status;
}