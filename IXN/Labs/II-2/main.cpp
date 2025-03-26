#include <IXN/Framework/imgui-opengl3.hpp>


class Ex1 {
public:
    inline static const char* credentials_file_name = "II_2_credentials.txt";

protected:
    inline static bool          _logged   = false;
    inline static std::string   _user     = "";
    inline static std::string   _pass     = "";

protected:
    static void _write_file( void ) {
        std::ofstream file{ credentials_file_name, std::fstream::out };
        file << "mip monster\nstefi fara-monster\n";
    }

protected:
    static ixN::DWORD _login_loop( double elapsed ) {
        static ixN::Ticker   tick{ ixN::ticker_lap_epoch_init_t{} };

        static int _constructor = ( [] () static -> int {
            _write_file();

            return 0;
        } )();


        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiWindowFlags_None );
        ImGui::Begin( "Ex1 - Log in..." );

        ImGui::SeparatorText( "LOGIN" );
        ImGui::InputTextWithHint( "User", "Your username.", &_user );
        ImGui::InputTextWithHint( "Password", "Your password.", &_pass, ImGuiInputTextFlags_Password );

        ImGui::Separator();
        if( ImGui::Button( "Log in" ) ) {
            std::ifstream file{ credentials_file_name };
            struct { std::string user, pass; } entry;

            while( file >> entry.user >> entry.pass ) {
                if( entry.user ==  _user && entry.pass == _pass ) {
                    _logged = true;
                    goto l_end;
                }
            }
            tick.lap();
        }
        
        if( auto rem = 3.0 - tick.peek_lap(); rem > 0.0 ) {
            ImGui::SameLine();
            ImGui::TextColored( { 1, 0, 0, ( float )rem / 3.0f }, "Invalid credentials." );
            ImGui::SetItemTooltip( "Disappearing in %.1fs...", rem );
        }

    l_end:
        ImGui::End();
        return 0;
    }

    static ixN::DWORD _logged_loop( double elapsed ) {
        static int _constructor = ( [] () static -> int {
            return 0;
        } )();


        ImGui::SetNextWindowSize( { 0, 0 }, ImGuiWindowFlags_None );
        ImGui::Begin( "Ex1 - Logged in!" );

        ImGui::SeparatorText( "WELCOME" );
        ImGui::Text( "Hello, %s.", _user.c_str() );
        
        ImGui::Separator();
        if( ImGui::Button( "Exit" ) ) {
            goto l_exit;
        }
    
        ImGui::End();
        return 0;

    l_exit:
        ImGui::End();
        return -1;
    }

public:  
    static ixN::DWORD loop( double elapsed ) {
        return _logged ? _logged_loop( elapsed ) : _login_loop( elapsed );
    }
};


class Ex2 {

};


int main( int argc, char* argv[] ) {
    ixN::Fwk::ImGui_on_OpenGL3 fwk;

    fwk.params.title = "II-2";
    fwk.params.iconify = true;

    static ixN::Fwk::ImGui_on_OpenGL3::frame_callback_t loops[] = {
        Ex1::loop
    };

    fwk.loop = loops[ atoi( argv[ 1 ] ) ];
    
    fwk.params.is_running = true;
    return fwk.main( argc, argv );
}