/*
*/

#include <IXT/aritm.hpp>
#include <IXT/surface.hpp>
#include <IXT/render2.hpp>

using namespace IXT;

int main() {
    Clust2 clust{ ASSET_CLST2_TRIG_ECH_PATH };
    clust.scale_with( .4 );

    Surface surf{ "IXT-Fdl-clust2", Crd2{}, Vec2{ std::min( Env::w<2.>(), Env::h<2.>() ) }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID };

    Renderer2 render{ surf };

    while( !surf.down( SurfKey::ESC ) ) {
        render.charge().fill( RGBA{ 0, 0, 0, 255 } );

        for( auto ray_itr = clust.outter_ray_begin(); ray_itr != clust.outter_ray_end(); ++ray_itr ) {
            Vec2 v1 = ( *ray_itr ).origin;
            Vec2 v2 = ( *ray_itr ).drop();
            
            render.line( v1, v2, render[ clust.contains( surf.ptr_v() ) ? RENDERER2_DFT_SWEEP_BLUE : RENDERER2_DFT_SWEEP_GREEN ] );
        }

        render.splash();
    }
}