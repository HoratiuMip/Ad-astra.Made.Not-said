/*
*/

#include <iostream>

#include <IXT/surface.hpp>
#include <IXT/render2.hpp>
#include <IXT/env.hpp>

using namespace IXT;

int main() {
    Surface surface{ "RenderFondle", { 0, 0 }, { Env::w<2.>(), Env::h<2.>() }, SURFACE_STYLE_LIQUID };
    surface.uplink( SURFACE_THREAD_ACROSS );
    Renderer2 render{ surface };


    while( !surface.down( SurfKey::ESC ) ) {
        render.charge().fill( RGBA{ 0 } );

        render
        .line( Vec2{ -.5, .5 }, surface.ptr_vg(), render.pull( RENDERER2_DEF_BRUSH_GREEN ) )
        .line( Vec2{ .5, .5 }, surface.ptr_vg(), render.pull( RENDERER2_DEF_BRUSH_GREEN ) )
        .line( Vec2{ .5, -.5 }, surface.ptr_vg(), render.pull( RENDERER2_DEF_BRUSH_GREEN ) )
        .line( Vec2{ -.5, -.5 }, surface.ptr_vg(), render.pull( RENDERER2_DEF_BRUSH_GREEN ) );

        render.splash();
    }
}