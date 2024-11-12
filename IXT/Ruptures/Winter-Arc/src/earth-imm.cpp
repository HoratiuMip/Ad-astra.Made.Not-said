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
    Surface surf{ "Warc Earth Immersion", Crd2{}, Vec2{ Env::w<1.>(), Env::h<1.>() }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_SOLID };
    Renderer3 rend{ surf };

    Lens3 lens{ glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

    Shader3 shader_vert{ WARC_RUPTURE_IMM_EARTH_DIR"/shader.vert", SHADER3_PHASE_VERTEX };
    Shader3 shader_frag{ WARC_RUPTURE_IMM_EARTH_DIR"/shader.frag", SHADER3_PHASE_FRAGMENT };

    ShaderPipe3 shader_pipe{ shader_vert, shader_frag };
    shader_pipe.uplink();

    Uniform3< glm::vec3 > sun_pos{ shader_pipe, "u_sun_pos", glm::vec3( 100.0 ) };
    Uniform3< glm::mat4 > model{ shader_pipe, "u_model", glm::mat4( 1.0f ) };
    Uniform3< glm::mat4 > view{ shader_pipe, "u_view", lens.view() };
    Uniform3< glm::mat4 > proj{ shader_pipe, "u_projection", glm::perspective(glm::radians(55.0f), surf.aspect(), 0.1f, 1000.0f) };
    model.uplink(); view.uplink(); proj.uplink(); sun_pos.uplink();

    ekg::Model3D mod3D;
    mod3D.LoadModel( WARC_RUPTURE_IMM_EARTH_DIR"/earth.obj", WARC_RUPTURE_IMM_EARTH_DIR"/" );

    while( !surf.down( SurfKey::ESC ) ) {
        static Ticker ticker;
        auto elapsed = ticker.lap() * 60.0;


        if( surf.down_any( SurfKey::RIGHT, SurfKey::LEFT, SurfKey::UP, SurfKey::DOWN ) ) {
            if( surf.down( SurfKey::LSHIFT ) ) {
                lens.zoom( ( surf.down( SurfKey::UP ) - surf.down( SurfKey::DOWN ) ) * .02 * elapsed );
                lens.roll( ( surf.down( SurfKey::RIGHT ) - surf.down( SurfKey::LEFT ) ) * .03 * elapsed );
            } else 
                lens.spin( {
                    ( surf.down( SurfKey::RIGHT ) - surf.down( SurfKey::LEFT ) ) * .03 * elapsed,
                    ( surf.down( SurfKey::UP ) - surf.down( SurfKey::DOWN ) ) * .03 * elapsed
                } );
        }

        rend.clear();
        sun_pos.uplink( glm::rotate( sun_pos.get(), ( float )( .01 * elapsed ), glm::vec3{ 0, 1, 0 } ) );
        view.uplink( lens.view() );

        //glBindTexture(GL_TEXTURE_2D, text_2);
        mod3D.Draw( shader_pipe );

        rend.swap();
    }

    surf.downlink();

    return 0;
}


} };
