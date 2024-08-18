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
    Renderer2 renderer{ surface };


    while( !surface.down( SurfKey::ESC ) ) {
       
    }
}