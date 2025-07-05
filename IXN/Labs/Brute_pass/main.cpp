#include <IXN/Framework/imgui_on_opengl3.hpp>


std::string_view   G_sets[]   = {
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "0123456789",
    ".,/:!@#$%^&*()"
};


struct Combiner : ixN::Descriptor {
    IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Combiner" );

    Combiner( const char* d ) : desc{ d } {} 

    struct Params {
        std::string_view   pass;
        int                sets;
    };

    inline static std::atomic_bool   reset_flag{ false };

    const char*        desc             = "Combiner";
    char               buffer[ 256 ];
    std::atomic_bool   found            = false;
    bool               show             = false;

    virtual void brute( const Params& params ) = 0;

    void begin( const Params& params ) {
        if( SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL ) )
            ixN::comms( this, ixN::EchoLevel_Ok, "Priority of thread set to TIME_CRITICAL." );
        else
            ixN::comms( this, ixN::EchoLevel_Warning, "Could not set thread priority to TIME_CRITICAL." );

        this->brute( params );
    }

    virtual void reset( void ) {
        memset( buffer, 0, sizeof( buffer ) );
        found = false;
    }
};

struct Combiner_all : Combiner {
    IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Combiner_all" );
    Combiner_all() : Combiner{ "Toate combinatiile." } {}

    virtual void brute( const Params& params ) override {
        std::function< bool( int, int ) > itr_pos = [ & ] ( int pos, int lim ) -> bool {
            if( pos >= lim || Combiner::reset_flag ) return false;

            for( int set = 0; set <= 3; ++set ) {
                if( 0 == ( ( params.sets >> set ) & 1 ) ) continue;

                for( auto c : G_sets[ set ] ) {
                    Combiner::buffer[ pos ] = c;

                    if( pos == params.pass.length() - 1 ) {
                        if( params.pass == Combiner::buffer ) return true;
                    } else if( itr_pos( pos + 1, lim ) ) return true;
                }
            }

            return false;
        };

        for( int n = 1; n <= params.pass.length(); ++n )
            if( itr_pos( 0, n ) ) { Combiner::found = true; return; }
    }
};

struct Combiner_all_length : Combiner {
    IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Combiner_all_length" );
    Combiner_all_length() : Combiner{ "Toate combinatiile, lungime cunoscuta." } {}

    virtual void brute( const Params& params ) override {
        std::function< bool( int ) > itr_pos = [ & ] ( int pos ) -> bool {
            if( Combiner::reset_flag ) return false;

            for( int set = 0; set <= 3; ++set ) {
                if( 0 == ( ( params.sets >> set ) & 1 ) ) continue;

                for( auto c : G_sets[ set ] ) {
                    Combiner::buffer[ pos ] = c;

                    if( pos == params.pass.length() - 1 ) {
                        if( params.pass == Combiner::buffer ) return true;
                    } else if( itr_pos( pos + 1 ) ) return true;
                }
            }

            return false;
        };

        Combiner::found = itr_pos( 0 );
    }
};

struct Combiner_common : Combiner {
    IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Combiner_common" );
    Combiner_common() : Combiner{ "Combinatia cuvintelor uzuale." } {
        for( int year = 1980; year <= 2025; ++year )
            commons.emplace_back( std::to_string( year ) );
    }

    std::vector< std::string >   commons   = {
        "tudor", "teodor",
        "acasa", "email", "mail", "birou",
        "pufi", "bobita", "azor",
        "om", "femeie", "barbat", "copil", "prieten", "prietena", "familie", "mama", "tata", "frate", "sora",
        "casa", "usa", "fereastra", "scaun", "masa", "pat", "carte", "telefon", "cheie",
        "soare", "luna", "stea", "apa", "foc", "copac", "floare", "munte", "plaja",
        "zi", "noapte", "dimineata", "seara", "luna", "an", "ceas",
        "mancare", "apa", "paine", "lapte", "cafea", "ceai", "carne", "ou", "fruct",
        "drum", "oras", "sat", "scoala", "magazin", "masina", "avion", "tren",
        "123", "1", "2003", "2004", "2005", "2025", "567",
        "1234", "1111", "0000", "2024", "2025", "1900", "2000", "1990", "1989",
        "112", "911", "1010", "1212", "777", "999", "314", "007", "69", "86",
        "0800", "4321", "2468", "1357", "0101", "2001", "1001", "1918", "1980", "2020"
    };

    virtual void brute( const Params& params ) override {
        std::function< bool( std::string, int, int ) > itr_pos = [ & ] ( std::string base, int pos, int lim ) -> bool {
            if( pos >= lim || Combiner::reset_flag ) return false;

            for( auto c : commons ) {
                std::string crt = base + c; strcpy( Combiner::buffer, crt.c_str() );

                if( crt == params.pass ) return true;
                
                if( itr_pos( crt, pos + 1, lim ) ) return true;
            }

            return false;
        };

        for( int n = 1; n <= 4; ++n )
            if( itr_pos( "", 0, n ) ) { Combiner::found = true; return; }
    }
};

std::string   input_pass;
ixN::Ticker   input_ticker;
std::thread   brute_th;

std::list< std::unique_ptr< Combiner > >   combiners;

void brute_main( std::string pass ) {
    for( auto& comb : combiners ) comb->reset();

    int sets = 0b0000;
    for( int set = 0; set <= 3; ++set ) { for( auto c : G_sets[ set ] ) {
        if( pass.find_first_of( c ) != std::string_view::npos ) {
            sets |= ( 1 << set );
            break;
        }
    } }

    std::vector< std::jthread > ths; ths.reserve( combiners.size() );
    Combiner::Params params{
        pass: pass,
        sets: sets
    };
    for( auto& comb : combiners )
        ths.emplace_back( Combiner::begin, comb.get(), params );
}


int input_cb( ImGuiInputTextCallbackData* ) {
    Combiner::reset_flag = true;
    if( brute_th.joinable() ) brute_th.join();
    Combiner::reset_flag = false;

    input_ticker.lap();
    return 0;
}

ixN::DWORD loop( double elapsed, [[maybe_unused]]void* ) {
    ImGui::Begin( "##brute_pass", nullptr );
    
    ImGui::SeparatorText( "Parola" );
    ImGui::SetNextItemWidth( 300 );
    ImGui::InputText( "##your_password", &input_pass, ImGuiInputTextFlags_CallbackEdit, input_cb );

    ImGui::Separator();

    for( auto& comb : combiners )
        ImGui::Checkbox( comb->desc, &comb->show );

    static bool th_launched = false;
    if( float t = input_ticker.peek_lap() - 2.0; t >= 0.0 && !input_pass.empty() ) {
        if( !th_launched ) {
            brute_th = std::thread{ brute_main, input_pass };
            th_launched = true;
        }

        for( auto& comb : combiners ) {
            if( !comb->show ) continue;

            ImGui::SeparatorText( comb->desc );
            ImGui::TextColored( comb->found ? ImVec4{ 1, 0, 0, 1 } : ImVec4{ 0, 1, 0, 1 }, "> %s", comb->buffer );
        }

    } else {
        ImGui::Separator();
        ImGui::ProgressBar( -1.0f * ( float )ImGui::GetTime(), { 300, 20 }, "" );
        th_launched = false;
    }

    ImGui::End();
    return 0;
}


int main( int argc, char* argv[] ) {    
    combiners.emplace_back( new Combiner_all{} );
    combiners.emplace_back( new Combiner_all_length{} );
    combiners.emplace_back( new Combiner_common{} );

    ixN::Fwk::ImGui_on_OpenGL3 fwk;
    fwk.params.title = "Brute_pass";
    fwk.params.iconify = true;
    fwk.params.is_running = true;
    fwk.loop = loop;
    fwk.init_hold = false;
    return fwk.main( argc, argv );
}