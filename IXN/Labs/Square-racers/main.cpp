#include <IXN/Framework/imgui-opengl3.hpp>
#include <IXN/tempo.hpp>


#define BARRIER_1 500
#define BARRIER_2 1100
#define SQUEEZE_Y 400
std::atomic_bool in_enclosure{ false };


ImVec2 operator + ( const ImVec2& lhs, const ImVec2& rhs ) {
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}


void draw_map( ImDrawList* imdraw ) {
    auto org = ImGui::GetCursorScreenPos();
    ImVec2 upper[] = {
        { 0, 0 }, { 500, 0 }, { 750, 300 }, { 850, 300 }, { 1100, 0 }, { 1600, 0 }
    };
    ImVec2 lower[] = {
        { 0, 800 }, { 500, 800 }, { 750, 500 }, { 850, 500 }, { 1100, 800 }, { 1600, 800 }
    };
    for( auto& p : upper ) { p.x += org.x; p.y += org.y; }
    for( auto& p : lower ) { p.x += org.x; p.y += org.y; }
    imdraw->AddPolyline( upper, sizeof( upper ) / sizeof( ImVec2 ), IM_COL32_WHITE, ImDrawFlags_None, 4.0f );
    imdraw->AddPolyline( lower, sizeof( upper ) / sizeof( ImVec2 ), IM_COL32_WHITE, ImDrawFlags_None, 4.0f );

    imdraw->AddLine( org + ImVec2{ BARRIER_1, 200 }, org + ImVec2{ BARRIER_1, 600 }, IM_COL32_WHITE, 2.0f );
    imdraw->AddLine( org + ImVec2{ BARRIER_2, 200 }, org + ImVec2{ BARRIER_2, 600 }, IM_COL32_WHITE, 2.0f );
}


struct Square {
    inline static std::atomic_bool reset{ true };
    inline static float            _Kp   = 4.0;
    inline static float            _Ki   = 3.0;
    inline static std::atomic_bool _2pass{ false };

    ImVec2        pos;
    float         tary;
    ImU32         col;
    float         spd   = 160;
    ixN::Ticker   _tkr;
    std::thread   _th;

    void go() {
        tary = pos.y;

        float eint = 0.0;

        auto advance_to = [ this, &eint ] ( float lim ) -> bool {
            while( true ) {
                if( reset ) return false;

                double elapsed = _tkr.lap();

                float dx = spd * elapsed;
                if( pos.x + dx >= lim ) break;
                pos.x += dx; 

                float ey = tary - pos.y;
                eint += ey * elapsed;
                pos.y += ( _Kp * ey + _Ki * eint ) * elapsed;

                std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
            }
            pos.x = lim;
            return true;
        };

        _tkr.lap();
        if( !advance_to( BARRIER_1 ) ) return;
        float last_y = pos.y; 
        tary = SQUEEZE_Y;

        do {
            bool flag = false;
            bool acq = in_enclosure.compare_exchange_strong( flag, true );
            if( acq ) break;

            in_enclosure.wait( true );
            if( reset ) return;
        } while( true );

        _tkr.lap();
        if( !advance_to( BARRIER_2 ) ) return;

        tary = last_y;

        in_enclosure.store( false, std::memory_order_seq_cst );
        in_enclosure.notify_all();

        if( this->is( 0 ) ) {
            _2pass.wait( false );
        } else if( this->is( 2 ) ) {
            _2pass = true;
            _2pass.notify_one();
        }

        _tkr.lap();
        advance_to( 1500 );

    }

    ImVec2 get_tl( void ) const {
        return { pos.x - 40, pos.y - 40 };
    }
    ImVec2 get_br( void ) const {
        return { pos.x + 40, pos.y + 40 };
    }

    bool is( int idx ) const;

} SQUARES[ 4 ];

bool Square::is( int idx ) const {
    return this == &SQUARES[ idx ];
}

void reset_squares() {
    Square::reset = true;
    in_enclosure = false;
    in_enclosure.notify_all();
    Square::_2pass = true;
    Square::_2pass.notify_one();
    Square::_2pass = false;

    for( auto& s : SQUARES ) {
        if( s._th.joinable() ) s._th.join();
    }

    auto get_speed = [] ( int mod, int add ) -> float {
        return ( float )( rand() % mod + add );
    };

    SQUARES[ 0 ] = Square{
        pos: { 100, 150 },
        col: IM_COL32( 0, 255, 255, 255 ),
        spd: get_speed( 200, 100 )
    };
    SQUARES[ 1 ] = Square{
        pos: { 100, 325 },
        col: IM_COL32( 180, 0, 255, 255 ),
        spd: get_speed( 200, 100 )
    };
    SQUARES[ 2 ] = Square{
        pos: { 100, 475 },
        col: IM_COL32( 255, 180, 0, 255 ),
        spd: std::min( get_speed( 200, 100 ), SQUARES[ 0 ].spd )
    };

    SQUARES[ 3 ] = Square{
        pos: { 100, 650 },
        col: IM_COL32( 0, 255, 0, 255 ),
        spd: get_speed( 200, 100 )
    };

    Square::reset = false;
}

void go_squares() {
    reset_squares();
    for( auto& s : SQUARES ) {
        s._th = std::thread{ Square::go, &s };
    }
}


ixN::DWORD loop( double elapsed ) {
    ImGui::SetNextWindowSize( { 1620, 840 }, ImGuiCond_Always );
    ImGui::Begin( "Square Racers" );

    auto* imdraw = ImGui::GetWindowDrawList();
    draw_map( imdraw );

    auto org = ImGui::GetCursorScreenPos();
    for( auto& s : SQUARES ) {
        imdraw->AddRectFilled( org + s.get_tl(), org + s.get_br(), s.col, 0.1f, ImDrawFlags_None );
    }

    ImGui::End();

    ImGui::SetNextWindowSize( { 0, 0 }, ImGuiCond_Always );
    ImGui::Begin( "Command" );

    if( ImGui::Button( "GO" ) ) {
        go_squares();
    }
    if( ImGui::Button( "RESET" ) ) {
        reset_squares();
    }

    ImGui::Separator();

    ImGui::VSliderFloat( "Kp", { 100, 300 }, &Square::_Kp, 0.0, 6.0, "%.3f", ImGuiSliderFlags_None );
    ImGui::SameLine();
    ImGui::VSliderFloat( "Ki", { 100, 300 }, &Square::_Ki, 0.0, 6.0, "%.3f", ImGuiSliderFlags_None );

    ImGui::End();

    return 0;
}


int main( int argc, char* argv[] ) {
    srand( time( nullptr ) );
    reset_squares();

    ixN::Fwk::ImGui_on_OpenGL3 fwk;
    fwk.params.title = "Square Racers";
    fwk.params.iconify = true;
    fwk.params.is_running = true;
    fwk.loop = loop;
    return fwk.main( argc, argv );
}