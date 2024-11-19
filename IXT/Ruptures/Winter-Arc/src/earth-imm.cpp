#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

#include <warc/earth-imm.hpp>

namespace warc { namespace imm {


using namespace IXT;


int EARTH::main( int argc, char* argv[] ) {
    struct _IMM {
        _IMM() 
        : surf{ "Warc Earth Imm", Crd2{}, Vec2{ Env::w<1.>(), Env::h<1.>() }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_SOLID },
          rend{ surf },
          lens{ glm::vec3( .0f, 5.0, 15.0 ), glm::vec3( .0, .0, .0 ), glm::vec3( .0, 1.0, .0 ) },

          view{ "view", lens.view() },
          proj{ "proj", glm::perspective( glm::radians( 55.0f ), surf.aspect(), 0.1f, 1000.0f ) },

          sun{ "sun_pos", glm::vec3{ 110.0 } }
        {
            view.push( earth.mesh.pipe );
            proj.push( earth.mesh.pipe );
            sun.push( earth.mesh.pipe );
            earth.mesh.model.uplink();

            view.push( sat_noaa.mesh.pipe );
            proj.push( sat_noaa.mesh.pipe );
            sun.push( sat_noaa.mesh.pipe );
            sat_noaa.mesh.model.uplink_v( 
                glm::translate( glm::mat4{ 1.0 }, glm::vec3{ .0, .0, 1.2 } )
                *
                glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 0.01 } ) 
            );

            view.uplink_b();
            proj.uplink_b();
            sun.uplink_b();
        }

        Surface     surf;
        Renderer3   rend;
        Lens3       lens;

        Uniform3< glm::mat4 > view;
        Uniform3< glm::mat4 > proj;

        struct _EARTH {
            _EARTH() 
            : mesh{ WARC_RUPTURE_IMM_EARTH_DIR, "earth", MESH3_FLAG_MAKE_SHADING_PIPE }
            {}

            IXT::Mesh3   mesh;
        } earth;

        struct _SAT_NOAA {
            _SAT_NOAA()
            : mesh{ WARC_RUPTURE_IMM_SAT_NOAA_DIR, "sat_noaa", MESH3_FLAG_MAKE_SHADING_PIPE }
            {}

            IXT::Mesh3   mesh;
        } sat_noaa;

        Uniform3< glm::vec3 >   sun;
        
    } imm;


    while( !imm.surf.down( SurfKey::ESC ) ) {
        static Ticker ticker;
        auto elapsed = ticker.lap() * 60.0;

        if( imm.surf.down_any( SurfKey::RIGHT, SurfKey::LEFT, SurfKey::UP, SurfKey::DOWN ) ) {
            if( imm.surf.down( SurfKey::LSHIFT ) ) {
                imm.lens.zoom( ( imm.surf.down( SurfKey::UP ) - imm.surf.down( SurfKey::DOWN ) ) * .02 * elapsed );
                imm.lens.roll( ( imm.surf.down( SurfKey::RIGHT ) - imm.surf.down( SurfKey::LEFT ) ) * .03 * elapsed );
            } else 
                imm.lens.spin( {
                    ( imm.surf.down( SurfKey::RIGHT ) - imm.surf.down( SurfKey::LEFT ) ) * .09 * elapsed,
                    ( imm.surf.down( SurfKey::UP ) - imm.surf.down( SurfKey::DOWN ) ) * .09 * elapsed
                } );
        }
        
        imm.rend.clear( glm::vec4{ .0, .0, .0, 1.0 } );

        imm.sun.uplink_bv( glm::rotate( imm.sun.get(), ( float )( .01 * elapsed ), glm::vec3{ 0, 1, 0 } ) );
        imm.view.uplink_bv( imm.lens.view() );
        
        imm.earth.mesh.splash();
        imm.sat_noaa.mesh.splash();

        imm.rend.swap();
    }

    imm.surf.downlink();

    return 0;
}


} };
