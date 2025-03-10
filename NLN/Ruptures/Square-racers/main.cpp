#include "square-racers-imgui.hpp"

#include <NLN/tempo.hpp>


#define BARRIER_1 500
#define BARRIER_2 1100
std::atomic_bool in_enclosure{ false };


ImVec2 operator + ( const ImVec2& lhs, const ImVec2& rhs ) {
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}


void draw_map( ImDrawList* imdraw ) {
    auto org = ImGui::GetCursorScreenPos();
    ImVec2 upper[] = {
        { 0, 0 }, { 600, 0 }, { 600, 350 }, { 1000, 350 }, { 1000, 0 }, { 1600, 0 }
    };
    ImVec2 lower[] = {
        { 0, 800 }, { 600, 800 }, { 600, 450 }, { 1000, 450 }, { 1000, 800 }, { 1600, 800 }
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

    ImVec2        pos;
    ImU32         col;
    float         spd = 160;
    NLN::Ticker   _tkr;
    std::thread   _th;

    void go() {
        float this_y = pos.y;

        auto advance_to = [ this ] ( float lim ) -> void {
            while( !reset ) {
               float dx = spd * _tkr.lap();
               if( pos.x + dx >= lim ) break;
               pos.x += dx; 
               std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
            }
            pos.x = lim;
        };

        _tkr.lap();
        advance_to( BARRIER_1 );

        advance_to( BARRIER_2 );

        advance_to( 1500 );

    }

    ImVec2 get_tl() const {
        return { pos.x - 40, pos.y - 40 };
    }
    ImVec2 get_br() const {
        return { pos.x + 40, pos.y + 40 };
    }

} SQUARES[ 4 ];

void reset_squares() {
    Square::reset = true;
    for( auto& s : SQUARES ) {
        if( s._th.joinable() ) s._th.join();
    }

    SQUARES[ 0 ] = Square{
        pos: { 100, 150 },
        col: IM_COL32( 0, 255, 255, 255 ),
        spd: rand() % 200 + 100
    };
    SQUARES[ 1 ] = Square{
        pos: { 100, 325 },
        col: IM_COL32( 180, 0, 255, 255 ),
        spd: rand() % 200 + 100
    };
    SQUARES[ 2 ] = Square{
        pos: { 100, 475 },
        col: IM_COL32( 255, 180, 0, 255 ),
        spd: rand() % 200 + 100
    };
    SQUARES[ 3 ] = Square{
        pos: { 100, 650 },
        col: IM_COL32( 0, 255, 0, 255 ),
        spd: rand() % 200 + 100
    };

    Square::reset = false;
}

void go_squares() {
    reset_squares();
    for( auto& s : SQUARES ) {
        s._th = std::thread{ Square::go, &s };
    }
}


void loop() {
    ImGui::SetNextWindowSize( { 1620, 840 }, ImGuiCond_Always );
    ImGui::Begin( "Square Racers" );

    auto* imdraw = ImGui::GetWindowDrawList();
    draw_map( imdraw );

    auto org = ImGui::GetCursorScreenPos();
    for( auto& s : SQUARES ) {
        imdraw->AddRectFilled( org + s.get_tl(), org + s.get_br(), s.col, 0.1f, ImDrawFlags_None );
    }

    ImGui::End();

    ImGui::Begin( "Command" );
    if( ImGui::Button( "GO" ) ) {
        go_squares();
    }
    if( ImGui::Button( "RESET" ) ) {
        reset_squares();
    }
    ImGui::End();
}


int main( int argc, char* argv[] ) {
    srand( time( nullptr ) );
    reset_squares();

    SquareRacersImGui graphics;
    graphics.loop = loop;
    graphics.win_w = graphics.win_h = 50;
    return graphics.main( argc, argv );
}