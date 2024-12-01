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


static EARTH* _impl_earth = nullptr;
#define PEARTH ((EARTH*)_impl_earth)

static void* _impl_imm = nullptr;
#define PIMM ((_IMM*)_impl_imm)
#define PIMM_UFRM ((_IMM::_UFRM*)&PIMM->ufrm)

#define ELAPSED_ARGS_DECL double ela, double rela
#define ELAPSED_ARGS_CALL ela, rela


void EARTH::set_sat_pos_update_func( SatUpdateFunc func ) {
    _sat_update_func = func;
}

int EARTH::main( int argc, char* argv[] ) {
    _impl_earth = this;


    struct _IMM : Descriptor {
        _IMM() 
        : _impl_set{ this },
        
          surf{ "Warc Earth Imm", Crd2{}, Vec2{ Env::w<1.>(), Env::h<1.>() }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_SOLID },
          rend{ surf },
          lens{ glm::vec3( .0, .0, 10.0 ), glm::vec3( .0, .0, .0 ), glm::vec3( .0, 1.0, .0 ) },

          ufrm{},

          earth{}, galaxy{}, sats{}
        {
            ufrm.proj.uplink_b();
        }

        struct _IMPL_SET {
            _IMPL_SET( _IMM* ptr ) {
                _impl_imm = ( void* )ptr;
            }

            ~_IMPL_SET() {
                _impl_imm = nullptr;
            }
        } _impl_set;

        Surface     surf;
        Renderer3   rend;
        Lens3       lens;

        int         control_scheme   = 1;

        struct _UFRM {
            _UFRM()
            : view{ "view", PIMM->lens.view() },
              proj{ "proj", glm::perspective( glm::radians( 55.0f ), PIMM->surf.aspect(), 0.1f, 1000.0f ) },
              sun{ "sun_pos", glm::vec3{ 180.0 } },
              lens{ "lens_pos", glm::vec3{ 0 } },
              rtc{ "rtc", 0.0f }
            {}

            Uniform3< glm::mat4 >   view;
            Uniform3< glm::mat4 >   proj;
            Uniform3< glm::vec3 >   sun;
            Uniform3< glm::vec3 >   lens;
            Uniform3< glm::f32 >    rtc;
            
        } ufrm;

        struct _EARTH {
            _EARTH() 
            : mesh{ WARC_RUPTURE_IMM_ROOT_DIR"earth/", "earth", MESH3_FLAG_MAKE_SHADING_PIPE }
            {
                mesh.model.uplink_v( glm::rotate( glm::mat4{ 1.0 }, -PIf / 2.0f, glm::vec3{ 0, 1, 0 } ) * mesh.model.get() );

                mesh.pipe->pull( 
                    PIMM_UFRM->view, PIMM_UFRM->proj, 
                    PIMM_UFRM->sun, PIMM_UFRM->rtc 
                );
            }

            Mesh3   mesh;
        } earth;

        struct _GALAXY {
            _GALAXY()
            : mesh{ WARC_RUPTURE_IMM_ROOT_DIR"galaxy/", "galaxy", MESH3_FLAG_MAKE_SHADING_PIPE }
            {
                mesh.model = glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 200.0 } ) * mesh.model.get();
                mesh.model.uplink();

                mesh.pipe->pull( 
                    PIMM_UFRM->view, PIMM_UFRM->proj, 
                    PIMM_UFRM->rtc 
                );
            }

            Mesh3   mesh;
        } galaxy;

        struct _SATS {
            _SATS() 
            : noaa{ { sat::NORAD_ID_NOAA_15 }, { sat::NORAD_ID_NOAA_18 }, { sat::NORAD_ID_NOAA_19 } }
            {
                hth_update = std::thread{ th_update, this };
            }
 
            Ticker               tick;
            std::thread          hth_update;
            std::atomic< int >   required_update_count   { 0 };
            bool                 attempt_update          = true;

            struct _SAT_NOAA {
                _SAT_NOAA( sat::NORAD_ID nid )
                : norad_id{ nid },
                  mesh{ WARC_RUPTURE_IMM_ROOT_DIR"sat_noaa/", "sat_noaa", MESH3_FLAG_MAKE_SHADING_PIPE }
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

                    mesh.pipe->pull(
                        PIMM_UFRM->view,
                        PIMM_UFRM->proj,
                        PIMM_UFRM->sun,
                        PIMM_UFRM->lens
                    );
                }

                sat::NORAD_ID                 norad_id;     
                glm::mat4                     base_model;
                glm::vec3                     base_pos;
                Mesh3                         mesh;
                glm::vec3                     pos;
                std::deque< sat::POSITION >   pos_cnt;

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
                    if( pos_cnt.empty() ) return *this;

                    sat::POSITION& sat_pos = pos_cnt.front();
                    pos_cnt.pop_front();
                    
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

                _SAT_NOAA& splash( ELAPSED_ARGS_DECL ) {
                    mesh.splash();
                    return *this;
                }

            } noaa[ 3 ];

            void th_update() {
                while( !PIMM->surf.down( SurfKey::ESC ) ) {
                    required_update_count.wait( 0 );

                    if( required_update_count.load( std::memory_order_relaxed ) < 0 )
                        return;

                    for( auto& s : noaa ) {
                        if( !attempt_update ) goto l_end;

                        switch( PEARTH->_sat_update_func( s.norad_id, s.pos_cnt ) ) {
                            case EARTH_SAT_UPDATE_RESULT_OK: break;
                            case EARTH_SAT_UPDATE_RESULT_REJECT: break;
                            case EARTH_SAT_UPDATE_RESULT_RETRY: break;

                            case EARTH_SAT_UPDATE_RESULT_WAIT: {
                                attempt_update = false;
                                WARC_LOG_RT_THAT_WARNING( PEARTH ) << "Satellite position update request responded with \"WAIT\".";
                            break; }
                        }
                    }

                l_end:
                    required_update_count.fetch_sub( 1, std::memory_order_release );
                }
            }

            void join_th_update() {
                required_update_count = -1;
                required_update_count.notify_one();

                if( !hth_update.joinable() ) return; 

                hth_update.join();
            }
            
            _SATS& refresh( ELAPSED_ARGS_DECL ) {
                if( tick.cmpxchg_lap( 1.0 ) ) {
                    if( noaa[ 0 ].pos_cnt.empty() ) {
                        required_update_count.store( 1, std::memory_order_release );
                        required_update_count.notify_one();
                    } else {
                        noaa[ 0 ].advance_pos();
                        noaa[ 1 ].advance_pos();
                        noaa[ 2 ].advance_pos();
                    }
                }

                return *this;
            }

            _SATS& splash( ELAPSED_ARGS_DECL ) {
                for( auto& s : noaa )
                    s.splash( ELAPSED_ARGS_CALL );
                return *this;
            }

        } sats;


        _IMM& control( ELAPSED_ARGS_DECL ) {
            static int configd_control_scheme = 0;

            if( configd_control_scheme != control_scheme ) {
                surf.socket_unplug( this->xtdx() );
                switch( control_scheme ) {
                    case 1: {
                        surf.on< SURFACE_EVENT_SCROLL >( [ & ] ( Vec2 cursor, SURFSCROLL_DIRECTION dir, [[maybe_unused]]auto& ) -> void {
                            switch( dir ) {
                                case SURFSCROLL_DIRECTION_UP: lens.zoom( .02 * lens.l2t(), { 1.3, 8.2 } ); break;
                                case SURFSCROLL_DIRECTION_DOWN: lens.zoom( -.02 * lens.l2t(), { 1.3, 8.2 } ); break;
                            }
                        } );
                    break; }
                }
                configd_control_scheme = control_scheme;
            }


            if( surf.down( SurfKey::COMMA ) )
                rend.uplink_wireframe();
            if( surf.down( SurfKey::DOT ) )
                rend.downlink_wireframe();

            
            if( surf.down( SurfKey::RMB ) ) {
                static Vec2 vel     = {};
                static Vec2 lcv_cmp = {};

                Vec2 lcv = surf.ptr_pv();
                if( lcv == lcv_cmp ) goto l_end;

                Vec2 cd = surf.ptr_v() - lcv;
                cd *= -PEARTH->lens_sens * lens.l2t();

                lens.spin_ul( { cd.x, cd.y } );

                lcv_cmp = lcv;
            }

        l_end:
            return *this;
        }

        _IMM& refresh( ELAPSED_ARGS_DECL ) {
            ufrm.lens.uplink_bv( lens.pos );
            ufrm.view.uplink_bv( lens.view() );

            ufrm.rtc.uplink_bv( ufrm.rtc.get() + rela );
            ufrm.sun.uplink_bv( glm::rotate( ufrm.sun.get(), ( float )( .0012 * ela ), glm::vec3{ 0, 1, 0 } ) );

            sats.refresh( ELAPSED_ARGS_CALL );
            
            return *this;
        }

        _IMM& splash( ELAPSED_ARGS_DECL ) {            
            rend.downlink_face_culling();
            galaxy.mesh.splash();
            rend.uplink_face_culling();

            earth.mesh.splash();
            sats.splash( ELAPSED_ARGS_CALL );

            return *this;
        }
        
        _IMM& run_pipe( ELAPSED_ARGS_DECL ) {
            return this->control( ELAPSED_ARGS_CALL ).refresh( ELAPSED_ARGS_CALL ).splash( ELAPSED_ARGS_CALL );
        }

    } imm;


    Ticker tick;
   
    while( !imm.surf.down( SurfKey::ESC ) ) {
        double elapsed_raw = tick.lap();
        double elapsed = elapsed_raw * 60.0;
        
        imm.rend.clear( glm::vec4{ 0.1, 0.1, 0.1, 1.0 } );
        imm.run_pipe( elapsed, elapsed_raw );
        imm.rend.swap();
    }

    imm.sats.join_th_update();
    imm.surf.downlink();

    _impl_earth = nullptr;

    return 0;
}


} };
