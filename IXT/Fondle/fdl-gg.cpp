/*
*/

#include <atomic>
#include <functional>
#include <iostream>
#include <vector>

#include <IXT/surface.hpp>
#include <IXT/render2.hpp>
#include <IXT/env.hpp>
#include <IXT/tempo.hpp>

using namespace IXT;

int main() {
    Surface surface{ "RenderFondle", { 0, 0 }, { Env::w<2.>(), Env::h<2.>() }, SURFACE_STYLE_LIQUID };
    surface.uplink( SURFACE_THREAD_ACROSS );

    Renderer2 render{ surface };

    Viewport2 port{ render, Crd2{ 0.5 }, Vec2{ 0.5 } };
    port.uplink();


    struct Scenes : std::vector< std::function< void() > > {
        Scenes() {
            this->reserve( count );
        }

        const int16_t           count   = 1;
        std::atomic< size_t >   at      = 0;

        void invoke_at() {
            std::invoke( this->operator[]( at ) );
        }
    } scenes;

// Lines from corners of surface to pointer vie renderer.
    scenes.emplace_back( [ & ] () -> void {
        render.charge().fill( RGBA{ 0 } );

        render
        .line( Vec2{ -.5, .5 }, surface.ptr_vg(), render[ RENDERER2_DFT_BRUSH_GREEN ] )
        .line( Vec2{ .5, .5 }, surface.ptr_vg(), render[ RENDERER2_DFT_BRUSH_GREEN ] )
        .line( Vec2{ .5, -.5 }, surface.ptr_vg(), render[ RENDERER2_DFT_BRUSH_GREEN ] )
        .line( Vec2{ -.5, -.5 }, surface.ptr_vg(), render[ RENDERER2_DFT_BRUSH_GREEN ] );

        render.splash();
    } );

// Dephased sine lines via renderer.
    scenes.emplace_back( [ & ] () -> void {
        static Ticker ticker;

        render.charge().fill( RGBA{ 0 } );

        render.line(
            Vec2{ -.4, ( ggfloat_t )sin( ticker.up_time() * 2 ) / 2 },
            Vec2{ .4, ( ggfloat_t )sin( ticker.up_time() * 3 ) / 2 },
            render[ RENDERER2_DFT_BRUSH_BLUE ]
        );

        render.splash();
    } );


    surface.on< SURFACE_EVENT_KEY >( [ & ] ( SurfKey key, SURFKEY_STATE state, [[maybe_unused]] auto& ) -> void {
        if( state == SURFKEY_STATE_DOWN ) 
            return;

        auto at = scenes.at.load( std::memory_order_relaxed );
        switch( key ) {
            case SurfKey::M:
                if( ++at == scenes.size() )
                    at = 0;
                goto l_scene_at_store;
            case SurfKey::N:
                if( --at == std::numeric_limits< std::remove_reference_t< decltype( at ) > >::max() )
                    at = scenes.size() - 1;
l_scene_at_store:
                scenes.at.store( at, std::memory_order_release );
        }
    } );


    while( !surface.down( SurfKey::ESC ) ) {
        scenes.invoke_at();
    }
}