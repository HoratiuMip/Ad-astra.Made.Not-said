/*
*/
#include <IXT/surface.hpp>
#include <IXT/os.hpp>

#include <list>

using namespace IXT;



int main() {
    Surface surface{ "IXT Surface", Crd2{ 64 }, Vec2{ 512 }, SURFACE_STYLE_LIQUID };
    surface.uplink( SURFACE_THREAD_ACROSS );
     surface.downlink();
     std::this_thread::sleep_for( std::chrono::seconds{ 5 } );
     surface.uplink( SURFACE_THREAD_ACROSS );

    std::list< SurfKey > pressed{};

    auto initial_crs = OS::console.crs();

    while( !surface.down( SurfKey::ESC ) ) {
        OS::console.crs_at( initial_crs );
        
        std::cout << std::fixed << std::setprecision( 0 ) 
                  << "Surface Pointer X[ " << surface.pointer().x << " ], Y[ " << surface.pointer().y 
                  << std::setw( 10 ) << std::left << " ].";
    }
}