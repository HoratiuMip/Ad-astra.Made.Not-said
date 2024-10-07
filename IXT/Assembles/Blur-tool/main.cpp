/*
*/

#include <IXT/ring-0.hpp>

using namespace IXT;

int main( int argc, char* argv[] ) {
    Surface surf{ "Blur-tool", { .0, .0 }, { std::min( Env::w<3.>(), Env::h<3.>() ) }, SURFACE_STYLE_LIQUID };
    
    auto render_th = std::jthread{ [ & ] () -> void {
        Renderer2 render{ surf };

        RdlSweep2 sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .2, .2 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 90, 10, 230 }, .62 },
                Sweep2gcn_t{ { 10, 10, 20 }, .62 },
                Sweep2gcn_t{ { 10, 10, 20 }, 1.0 }
            }, surf.height() 
        };

        while( !surf.down( SurfKey::ESC ) ) {
            render.charge().fill( RGBA{ .0, .0, .0, 1.0 } );
            sweep.org_at( surf.ptr_v() );
            render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, sweep );
            render.splash();

            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }
    } };

    surf.on< SURFACE_EVENT_FILEDROP >( [ & ] ( std::vector< std::string > files, [[maybe_unused]]auto& ) -> void {

    } );

    surf.uplink( SURFACE_THREAD_THROUGH );
}