/*
*/

#include <atomic>
#include <functional>
#include <iostream>
#include <deque>

#include <IXT/surface.hpp>
#include <IXT/render2.hpp>
#include <IXT/env.hpp>

using namespace IXT;

int main() { 
    Surface surface{ "FDL-GG-Ray-X", { 0, 0 }, { Env::w<2.>(), Env::h<2.>() }, SURFACE_STYLE_LIQUID };
    surface.uplink( SURFACE_THREAD_ACROSS );

    Renderer2 render{ surface };

    std::deque< Ray2 > rays = { {
        Ray2{ { -.1, .1 }, { .2, .0 } },
        Ray2{ { -.1, -.1 }, { .2, .0 } }
    } };

    std::atomic< size_t > rays_at = 0;
    std::atomic< bool > ray_end = 0;

    surface.on< SURFACE_EVENT_POINTER >( [ & ] ( Vec2 ptr, [[maybe_unused]] Vec2, [[maybe_unused]] SurfaceTrace& ) -> void {
        size_t at = rays_at.load( std::memory_order_relaxed );
        bool end = ray_end.load( std::memory_order_relaxed );

        switch( end ) {
            case 0x0: rays[ at ][ end ] = ptr; break;
            case 0x1: rays[ at ][ end ] = ptr - rays[ at ][ 0 ]; break;
        }
    } );

    surface.on< SURFACE_EVENT_KEY >( [ & ] ( SurfKey key, SURFKEY_STATE state, [[maybe_unused]] SurfaceTrace& ) -> void {
        if( state != SURFKEY_STATE_UP ) goto l_not_up;

        {
        size_t at = rays_at.load( std::memory_order_relaxed );

        switch( key ) {
            case SurfKey::LMB: {
                rays_at.store( ( at == rays.size() - 1 ) ? ( 0 ) : ( at + 1 ), std::memory_order_relaxed );
            break; }

            case SurfKey::RMB: {
                ray_end.store( ray_end.load( std::memory_order_relaxed ) ^ 1, std::memory_order_relaxed );
            break; }
        }
        }

l_not_up:
        return;
    } );

    while( true ) {
        render.charge().fill( RGBA{ .04, .04, .06, 1.0 } );

        for( auto& ray : rays ) {
            auto sweep = RENDERER2_DFT_SWEEP_BLUE;

            for( auto& cX : rays ) {
                auto rez = ray.X< Vec2 >( cX );

                if( rez.has_value() ) {
                    render.line( Vec2::O(), rez.value(), render[ RENDERER2_DFT_SWEEP_MAGENTA ] );
                }

                if( &cX != &ray && ray.X< bool >( cX ) ) {
                    sweep = RENDERER2_DFT_SWEEP_RED;
                    break;
                }
            }

            render.line( ray.origin, ray.drop(), render[ sweep ] );
        }

        render.splash();
    }

}