/*
*/

#include <IXT/ring-0.hpp>

#include <deque>

using namespace IXT;

int main( int argc, char* argv[] ) {
    Surface surf{ "Blur-tool", { .0, .0 }, { std::min( Env::w<2.>(), Env::h<2.>() ) }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID };

    Endec::Bmp           bmp  = {};
    std::deque< Clust2 > regz = {};
    std::mutex           sync = {};
    std::atomic< bool >  sreq = { false };
    
    auto render_th = std::thread{ [ & ] () -> void {
        Renderer2 render{ surf };

        RdlSweep2 inv_sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .12, .12 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 230, 10, 10 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, 1.0 }
            }, surf.height() 
        };

        RdlSweep2 bmp_sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .12, .12 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 90, 230, 10 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, 1.0 }
            }, surf.height() 
        };

        RdlSweep2 reg_sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .12, .12 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 90, 10, 230 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, 1.0 }
            }, surf.height() 
        };

        while( !surf.down( SurfKey::ESC ) ) {
            render.charge().fill( RGBA{ 10, 10, 20 } );

            if( bmp.buffer == nullptr ) {
                inv_sweep.org_at( surf.ptr_v() );
                render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, inv_sweep );
                goto l_end;
            }
            {
            Vec2 ref = surf.ptr_v();

            sreq.wait( true, std::memory_order_relaxed );
            std::unique_lock lock{ sync };

            ggfloat_t theta_step = 360.0 / ( regz.size() + 1 );
            for( int n = 1; n <= regz.size(); ++n ) {
                reg_sweep.org_at( ref.spinned( theta_step * n ) );
                render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, reg_sweep );
            }

            bmp_sweep.org_at( ref );
            render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, bmp_sweep );
            }
l_end:
            render.splash();
        }
    } };

    surf.on< SURFACE_EVENT_FILEDROP >( [ & ] ( std::vector< std::string > files, [[maybe_unused]]auto& ) -> void {
        sreq.store( true, std::memory_order_relaxed );
        std::unique_lock lock{ sync };

        for( auto& file : files ) {
            if( file.ends_with( ".clst2" ) ) {
                regz.emplace_back( file );
            } else if( file.ends_with( ".bmp" ) ){
                bmp.~Bmp();
                new ( &bmp ) Endec::Bmp{ file };
            }
        }

        sreq.store( false, std::memory_order_relaxed );
        sreq.notify_one();
    } );

    while( !surf.down( SurfKey::ESC ) ) {
        std::this_thread::sleep_for( std::chrono::milliseconds{ 1000 } );
    }

    render_th.join();
}