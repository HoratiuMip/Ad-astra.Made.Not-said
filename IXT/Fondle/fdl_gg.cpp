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

    SolidBrush2 brush{ render, RGBA{ 0, 0, 255 }, 3 };


    while( !surface.down( SurfKey::ESC ) ) {
        render.charge().fill( RGBA{ 0 } );

        render
        .line( Vec2{ -.5, .5 }, surface.ptr_vg(), brush )
        .line( Vec2{ .5, .5 }, surface.ptr_vg(), brush )
        .line( Vec2{ .5, -.5 }, surface.ptr_vg(), brush )
        .line( Vec2{ -.5, -.5 }, surface.ptr_vg(), brush );

        render.splash();
    }
}