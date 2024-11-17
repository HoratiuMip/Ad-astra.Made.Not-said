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
    Surface surf{ "Warc Earth Imm", Crd2{}, Vec2{ Env::w<1.>(), Env::h<1.>() }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_SOLID };
    Renderer3 rend{ surf };

    Lens3 lens{ glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

    ShaderPipe3 pipe{ 
        Shader3{ WARC_RUPTURE_IMM_EARTH_DIR"/earth_shader.vert", SHADER3_PHASE_VERTEX }, 
        Shader3{ WARC_RUPTURE_IMM_EARTH_DIR"/earth_shader.frag", SHADER3_PHASE_FRAGMENT }
    };
    pipe.uplink();

    Uniform3< glm::vec3 > sun_pos{ pipe, "u_sun_pos", glm::vec3( 100.0 ) };
    Uniform3< glm::mat4 > model{ pipe, "u_model", glm::mat4( 1.0f ) };
    Uniform3< glm::mat4 > view{ pipe, "u_view", lens.view() };
    Uniform3< glm::mat4 > proj{ pipe, "u_projection", glm::perspective(glm::radians(55.0f), surf.aspect(), 0.1f, 1000.0f) };
    model.uplink(); view.uplink(); proj.uplink(); sun_pos.uplink();


    struct _IMM {
        struct _EARTH {
            _EARTH() 
            : mesh{ WARC_RUPTURE_IMM_EARTH_DIR"/earth.obj", WARC_RUPTURE_IMM_EARTH_DIR"/" }
            {}

            IXT::Mesh3   mesh;
        } earth;

        struct _SAT_NOAA {
            _SAT_NOAA()
            : mesh{ WARC_RUPTURE_IMM_SAT_NOAA_DIR"/sat_noaa.obj", WARC_RUPTURE_IMM_SAT_NOAA_DIR"/" }
            {}

            IXT::Mesh3   mesh;
        } sat_noaa;
        
    } imm;

    imm.earth.mesh.dock_in( pipe );
    imm.sat_noaa.mesh.dock_in( pipe );

    while( !surf.down( SurfKey::ESC ) ) {
        static Ticker ticker;
        auto elapsed = ticker.lap() * 60.0;


        if( surf.down_any( SurfKey::RIGHT, SurfKey::LEFT, SurfKey::UP, SurfKey::DOWN ) ) {
            if( surf.down( SurfKey::LSHIFT ) ) {
                lens.zoom( ( surf.down( SurfKey::UP ) - surf.down( SurfKey::DOWN ) ) * .02 * elapsed );
                lens.roll( ( surf.down( SurfKey::RIGHT ) - surf.down( SurfKey::LEFT ) ) * .03 * elapsed );
            } else 
                lens.spin( {
                    ( surf.down( SurfKey::RIGHT ) - surf.down( SurfKey::LEFT ) ) * .09 * elapsed,
                    ( surf.down( SurfKey::UP ) - surf.down( SurfKey::DOWN ) ) * .09 * elapsed
                } );
        }

        rend.clear( glm::vec4{ .1, .1, .1, 1.0 } );
        sun_pos.uplink( glm::rotate( sun_pos.get(), ( float )( .01 * elapsed ), glm::vec3{ 0, 1, 0 } ) );
        view.uplink( lens.view() );

        pipe.uplink();
        imm.sat_noaa.mesh.splash();
        imm.earth.mesh.splash();

        //sat_noaa.mesh.splash();

        rend.swap();
    }

    surf.downlink();

    return 0;
}


} };
