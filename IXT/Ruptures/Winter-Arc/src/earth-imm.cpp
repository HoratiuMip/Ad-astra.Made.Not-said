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
          lens{ glm::vec3( .0, .0, 10.0 ), glm::vec3( .0, .0, .0 ), glm::vec3( .0, 1.0, .0 ) },

          view{ "view", lens.view() },
          proj{ "proj", glm::perspective( glm::radians( 55.0f ), surf.aspect(), 0.1f, 1000.0f ) },
          perlin_fac{ "perlin_fac", 0.0 },

          sun{ "sun_pos", glm::vec3{ 180.0 } },
          ufrm_lens_pos{ "lens_pos", glm::vec3{ 0 } },
          ufrm_rtc{ "rtc", 0.0f }
        {
            earth.mesh.pipe->pull( view, proj, sun, perlin_fac );
            galaxy.mesh.pipe->pull( view, proj, ufrm_rtc );
            sat_noaa[ 0 ].mesh.pipe->pull( view, proj, sun, ufrm_lens_pos );
            sat_noaa[ 1 ].mesh.pipe->pull( view, proj, sun, ufrm_lens_pos );
            sat_noaa[ 2 ].mesh.pipe->pull( view, proj, sun, ufrm_lens_pos );

            view.uplink_b();
            proj.uplink_b();
            sun.uplink_b();
        }

        Surface     surf;
        Renderer3   rend;
        Lens3       lens;

        Uniform3< glm::mat4 >   view;
        Uniform3< glm::mat4 >   proj;
        Uniform3< glm::f32 >    perlin_fac;

        Uniform3< glm::vec3 >   sun;
        Uniform3< glm::vec3 >   ufrm_lens_pos;
        Uniform3< glm::f32 >    ufrm_rtc;

        struct _EARTH {
            _EARTH() 
            : mesh{ WARC_RUPTURE_IMM_ROOT_DIR"earth/", "earth", MESH3_FLAG_MAKE_SHADING_PIPE }
            {
                mesh.model.uplink_v( glm::rotate( glm::mat4{ 1.0 }, -PIf / 2.0f, glm::vec3{ 0, 1, 0 } ) * mesh.model.get() );
            }

            IXT::Mesh3   mesh;
        } earth;

        struct _GALAXY {
            _GALAXY()
            : mesh{ WARC_RUPTURE_IMM_ROOT_DIR"galaxy/", "galaxy", MESH3_FLAG_MAKE_SHADING_PIPE }
            {
                mesh.model = glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 200.0 } ) * mesh.model.get();
                mesh.model.uplink();
            }

            IXT::Mesh3   mesh;
        } galaxy;

        struct _SAT_NOAA {
            _SAT_NOAA()
            : mesh{ WARC_RUPTURE_IMM_ROOT_DIR"sat_noaa/", "sat_noaa", MESH3_FLAG_MAKE_SHADING_PIPE }
            {
                pos = base_pos = glm::vec4{ .0, .0, 1.086, 1.0 };
                base_model = glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 1.0 / 21.8 / 13.224987 } );
                mesh.model.uplink_v( 
                    glm::translate( glm::mat4{ 1.0 }, pos ) 
                    *
                    glm::rotate( glm::mat4{ 1.0 }, glm::radians( 180.0f ), glm::vec3{ 0, 0, 1 } ) 
                    *
                    base_model 
                );
            }

            glm::mat4    base_model;
            IXT::Mesh3   mesh;
            glm::vec3    base_pos;
            glm::vec3    pos;

            std::deque< sat::POSITION >   global_positions;

            _SAT_NOAA& pos_to( glm::vec3 n_pos ) {
                mesh.model.uplink_v( 
                    glm::inverse( glm::lookAt( n_pos, glm::vec3{ 0.0 }, n_pos - pos ) )
                    *
                    glm::rotate( glm::mat4{ 1.0 }, PIf, glm::vec3{ 0, 0, 1 } )
                    *
                    base_model 
                );

                pos = n_pos;
                return *this;            
            }

            _SAT_NOAA& advance_pos() {
                if( global_positions.empty() ) return *this;

                sat::POSITION& sat_pos = global_positions.front();
                global_positions.pop_front();
                
                return this->pos_to( this->translate_latlong_2_glpos( sat_pos ) );
            }

            glm::vec3 translate_latlong_2_glpos( const sat::POSITION& sat_pos ) {
                float rx = glm::radians( sat_pos.satlatitude );
                float ry = glm::radians( sat_pos.satlongitude );

                return glm::rotate( glm::mat4{ 1.0 }, ry, glm::vec3{ 0, 1, 0 } )
                       *
                       glm::rotate( glm::mat4{ 1.0 }, -rx, glm::vec3{ 1, 0, 0 } )
                       *
                       glm::vec4{ base_pos, 1.0 };
            }

        } sat_noaa[ 3 ];


        void splash( double elapsed ) {
            rend.clear( glm::vec4{ .0, .0, .0, 1.0 } );
           
            ufrm_lens_pos.uplink_v( lens.pos );
            view.uplink_bv( lens.view() );
            
            rend.downlink_face_culling();
            galaxy.mesh.splash();
            rend.uplink_face_culling();

            earth.mesh.splash();
            sat_noaa[ 0 ].mesh.splash();
            sat_noaa[ 1 ].mesh.splash();
            sat_noaa[ 2 ].mesh.splash();

            rend.swap();
        }
        
    } imm;

    Ticker tick_rt;
    Ticker tick_sat_pos;
   
    while( !imm.surf.down( SurfKey::ESC ) ) {
        double elapsed_raw = tick_rt.lap();
        double elapsed = elapsed_raw * 60.0;

        imm.ufrm_rtc.uplink_v( imm.ufrm_rtc.get() + elapsed_raw );

        if( tick_sat_pos.cmpxchg_lap( 1.0 ) ) { 
            if( imm.sat_noaa[ 0 ].global_positions.empty() ) {
                this->_sat_update_func( sat::NORAD_ID_NOAA_15, imm.sat_noaa[ 0 ].global_positions, 180 );
                this->_sat_update_func( sat::NORAD_ID_NOAA_18, imm.sat_noaa[ 1 ].global_positions, 180 );
                this->_sat_update_func( sat::NORAD_ID_NOAA_19, imm.sat_noaa[ 2 ].global_positions, 180 );
            }
            imm.sat_noaa[ 0 ].advance_pos();
            imm.sat_noaa[ 1 ].advance_pos();
            imm.sat_noaa[ 2 ].advance_pos();
        }

        if( imm.surf.down_any( SurfKey::RIGHT, SurfKey::LEFT, SurfKey::UP, SurfKey::DOWN ) ) {
            if( imm.surf.down( SurfKey::LSHIFT ) ) {
                imm.lens.zoom( ( imm.surf.down( SurfKey::UP ) - imm.surf.down( SurfKey::DOWN ) ) * .02 * elapsed );
                imm.lens.roll( ( imm.surf.down( SurfKey::RIGHT ) - imm.surf.down( SurfKey::LEFT ) ) * .03 * elapsed );
            } else 
                imm.lens.spin( {
                    ( imm.surf.down( SurfKey::RIGHT ) - imm.surf.down( SurfKey::LEFT ) ) * .03 * elapsed,
                    ( imm.surf.down( SurfKey::UP ) - imm.surf.down( SurfKey::DOWN ) ) * .03 * elapsed
                } );
        }
        
        imm.perlin_fac.uplink_v( 22.2 * sin( tick_rt.up_time() / 14.6 ) );
        imm.sun.uplink_bv( glm::rotate( imm.sun.get(), ( float )( .001 * elapsed ), glm::vec3{ 0, 1, 0 } ) );
        
        imm.splash( elapsed );
    }

    imm.surf.downlink();

    return 0;
}


} };
