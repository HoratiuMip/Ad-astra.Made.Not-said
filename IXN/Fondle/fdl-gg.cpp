/*
*/

#include <atomic>
#include <functional>
#include <iostream>
#include <vector>

#include <IXN/surface.hpp>
#include <IXN/render2.hpp>
#include <IXN/env.hpp>
#include <IXN/tempo.hpp>

using namespace ixN;

int main() {
    Surface surface{ "RenderFondle", { 0, 0 }, { Env::w( .5 ), Env::h( .5 ) }, SURFACE_STYLE_LIQUID };
    surface.uplink( SURFACE_THREAD_ACROSS );

    Renderer2 render{ surface };

    Viewport2 port{ render, Crd2{ .2 }, Vec2{ .6 } };
    port.srf_uplink();

    Viewport2 port2{ port, Crd2{ .3 }, Vec2{ 1.0 } };
    port2.srf_uplink();

    Sprite2 rammus{ render, ASSET_PNG_RAMMUS_PATH };


    struct Scenes : std::vector< std::function< void() > > {
        Scenes() {
            this->reserve( count );
        }

        const int16_t           count   = 2;
        std::atomic< size_t >   at      = 0;

        void invoke_at() {
            std::invoke( this->operator[]( at ) );
        }
    } scenes;

//# Lines from corners of surface to pointer vie renderer.
    scenes.emplace_back( [ & ] () -> void {
        static LnrSweep2 sweep1{
            render, { -.5, .5 }, { .5, -.5 }, {
                sweep2_gc_node_t{ { 255, 0, 0, 255 }, 0.0 },
                sweep2_gc_node_t{ { 255, 255, 255, 255 }, 1.0 }
            }, 0.02
        };
        static RdlSweep2 sweep2{
            render, { .0, .0 }, { .0, .0 }, { .5, .5 }, {
                sweep2_gc_node_t{ { 255, 0, 0, 255 }, 0.0 },
                sweep2_gc_node_t{ { 255, 255, 255, 255 }, 1.0 }
            }, 0.02
        };

        render.rs2_uplink().fill( RGBA{ 0 } );

        sweep2.org_at( surface.ptr_v() );

        Sweep2& crt_sweep = surface.down( SurfKey::LMB ) ? ( Sweep2& )sweep1 : ( Sweep2& )sweep2;

        render
        .line( Vec2{ -.5, .5 }, surface.ptr_v(), crt_sweep )
        .line( Vec2{ .5, .5 }, surface.ptr_v(), crt_sweep )
        .line( Vec2{ .5, -.5 }, surface.ptr_v(), crt_sweep )
        .line( Vec2{ -.5, -.5 }, surface.ptr_v(), crt_sweep );

        render.rs2_downlink();
    } );

//# Dephased sine lines via renderer.
    scenes.emplace_back( [ & ] () -> void {
        static Ticker                    ticker          = {};
        static constexpr size_t          arr_sz          = 122;
        static std::pair< Vec2, Vec2 >   arr[ arr_sz ]   = {};
        static size_t                    arr_at          = 0;
        static ggfloat_t                 a_step          = 1.0 / arr_sz;
        static RdlSweep2                 sweep           = { 
            render, { .0, .0 }, { .0, .0 }, { .5, .5 }, {
                sweep2_gc_node_t{ { 80, 10, 255 }, .0 },
                sweep2_gc_node_t{ { 255, 10, 80 }, 1.0 }
            }, 0.01
        };

        sweep.org_at( port2.ptr_v() );

        render.rs2_uplink().fill( RGBA{ 0 } );

        auto [ left_new, right_new ] = std::make_pair< Vec2, Vec2 >( 
            { -.4, ( ggfloat_t )sin( ticker.up_time() * 2 ) / 2 },
            { .4, ( ggfloat_t )sin( ticker.up_time() * 3 ) / 2 }
        );

        const bool port_res = surface.down( SurfKey::RMB );

        port.rs2_uplink(); if( port_res ) port.restrict();
        port2.rs2_uplink();
        port2.line( left_new, right_new, sweep );

        float  a_at = 1.0;
        size_t at   = arr_at;
        for( int8_t n = 1; n <= arr_sz; ++n ) {
            auto [ left, right ] = arr[ at-- ];

            sweep.a( a_at -= a_step );
            port2.line( left, right, sweep );

            if( at == ~0ULL )
                at = arr_sz - 1;
        }

        sweep.a( 1.0 );

        if( ticker.cmpxchg_lap( 0.008 ) ) {
            if( ++arr_at == arr_sz )
                arr_at = 0;

            arr[ arr_at ] = { left_new, right_new };
        }
      
        port2.splash_bounds();
        port2.rs2_downlink();
        port.splash_bounds();
        if( port_res ) port.lift_restrict();
        port.rs2_downlink();
        render.rs2_downlink();
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