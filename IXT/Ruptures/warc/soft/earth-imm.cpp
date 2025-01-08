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
#define PEARTH_PARAMS ((EARTH_PARAMS*)(_impl_earth->_params.get()))

static void* _impl_imm = nullptr;
#define PIMM ((_IMM*)_impl_imm)
#define PIMM_UFRM ((_IMM::_UFRM*)&PIMM->ufrm)
#define PIMM_CTRL ((_IMM::_CTRL*)&PIMM->ctrl)


struct _IMM : Descriptor {
    _IMM() 
    : _impl_set{ this },
    
        surf{ "Warc Earth Imm", Crd2{}, Vec2{ Env::w<1.>(), Env::h<1.>() }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_SOLID },
        rend{ surf },
        lens{ glm::vec3( .0, .0, 8.2 ), glm::vec3( .0, .0, .0 ), glm::vec3( .0, 1.0, .0 ) },

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

    Surface   surf;
    Render3   rend;
    Lens3     lens;

    int      cinematic        = 0;
    Ticker   cinematic_tick   = {};
    float    cinematic_wy     = 0.24;

    struct _UFRM {
        _UFRM()
        : view{ "view", PIMM->lens.view() },
          proj{ "proj", glm::perspective( glm::radians( PEARTH_PARAMS->lens_fov ), PIMM->surf.aspect(), 0.1f, 1000.0f ) },
          lens_pos{ "lens_pos", glm::vec3{ 0 } },
          rtc{ "rtc", 0.0f },
          sat_high{ "sat_high", 0.0f }
        {}

        Uniform3< glm::mat4 >   view;
        Uniform3< glm::mat4 >   proj;
        Uniform3< glm::vec3 >   lens_pos;
        Uniform3< glm::f32 >    rtc;
        Uniform3< glm::f32 >    sat_high;
        
    } ufrm;


    struct _SUN {
        _SUN()
        : pos{ "sun_pos", glm::vec3{ 0.0, 0.0, 180.0 } }
        {
            this->_load_rt_pos();
        }

        Ticker                  tick;
        Uniform3< glm::vec3 >   pos;

        void _load_rt_pos() {
            auto ll = astro::sun_lat_long_now();
            pos.uplink_bv( glm::vec3{ astro::nrm_from_lat_long( ll ) * 180.0f } );
        }

        _SUN& refresh( float elapsed ) {
            if( tick.cmpxchg_lap( 60.0 ) ) {
                this->_load_rt_pos();
            }

            return *this;
        }

    } sun;

    struct _EARTH {
        _EARTH() 
        : mesh{ WARC_IMM_ROOT_DIR/"earth/", "earth", MESH3_FLAG_MAKE_PIPES },
          sat_poss{ "sat_poss" },
          sat_high_specs{ "sat_high_specs" },
          countries{ "show_countries", 0 }
        {
            mesh.model.uplink_v( glm::rotate( glm::mat4{ 1.0 }, -PIf / 2.0f, glm::vec3{ 0, 1, 0 } ) * mesh.model.get() );

            mesh.pipe->pull( 
                PIMM_UFRM->view, PIMM_UFRM->proj,
                PIMM_UFRM->lens_pos, 
                PIMM_UFRM->rtc,
                PIMM_UFRM->sat_high,
                PIMM->sun.pos,
                this->sat_poss, this->sat_high_specs, this->countries
            );

            PIMM->sun.pos.uplink_b();
        }

        Mesh3                        mesh;
        Uniform3< glm::vec3[ 3 ] >   sat_poss;
        Uniform3< glm::vec3[ 3 ] >   sat_high_specs;
        Uniform3< glm::i32 >         countries;

    } earth;

    struct _GALAXY {
        _GALAXY()
        : mesh{ WARC_IMM_ROOT_DIR/"galaxy/", "galaxy", MESH3_FLAG_MAKE_PIPES }
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

            for( auto& s : noaa ) {
                s.mesh.pipe->pull( PIMM_UFRM->sat_high );
            }
        }

        Ticker            tick;
        std::thread       hth_update;
        std::atomic_int   required_update_count   = 0;

        struct _SAT_NOAA {
            static constexpr float BASE_ELEVATION = 1.086;

            _SAT_NOAA( sat::NORAD_ID nid )
            : norad_id{ nid },
                mesh{ WARC_IMM_ROOT_DIR/"sat_noaa/", "sat_noaa", MESH3_FLAG_MAKE_PIPES },
                high_spec{ "high_spec", glm::vec3{ 0.0 } }
            {
                pos = glm::vec4{ .0, .0, BASE_ELEVATION, 1.0 };
                base_model = glm::scale( glm::mat4{ 1.0 }, glm::vec3{ 1.0 / 21.8 / 13.224987 } );
                mesh.model.uplink_v( 
                    glm::translate( glm::mat4{ 1.0 }, pos ) 
                    *
                    glm::rotate( glm::mat4{ 1.0 }, glm::radians( 180.0f ), glm::vec3{ 0, 0, 1 } ) 
                    *
                    base_model 
                );

                switch( this->norad_id ) {
                    case sat::NORAD_ID_NOAA_15: base_high_spec = glm::vec3( 0.001, 0.8, 0.4 ); break;
                    case sat::NORAD_ID_NOAA_18: base_high_spec = glm::vec3( 0.4, 0.001, 0.8 ); break;
                    case sat::NORAD_ID_NOAA_19: base_high_spec = glm::vec3( 0.8, 0.4, 0.001 ); break;

                    default: base_high_spec = glm::vec3( 1.0, 0.0, 0.82 ); break;
                }

                mesh.pipe->pull(
                    PIMM_UFRM->view,
                    PIMM_UFRM->proj,
                    PIMM_UFRM->lens_pos,
                    PIMM->sun.pos,
                    this->high_spec
                );
            }

            sat::NORAD_ID                 norad_id;     

            glm::mat4                     base_model;
            glm::vec3                     base_high_spec;
            Mesh3                         mesh;
            glm::vec3                     pos;
            Uniform3< glm::vec3 >         high_spec;

            std::mutex                    pos_cnt_mtx;
            std::deque< sat::POSITION >   pos_cnt;
            std::atomic_int               pos_cnt_update_required   = false;
            std::atomic_bool              hold_update               = false;

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
                return astro::nrm_from_lat_long( astro::LAT_LONG{ ( float )sat_pos.satlatitude, ( float )sat_pos.satlongitude } ) * BASE_ELEVATION;
            }

            _SAT_NOAA& splash( float elapsed ) {
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
                    if( s.hold_update.load( std::memory_order_relaxed ) ) {
                        WARC_ECHO_RT_THAT_INTEL( PEARTH ) << "Satellite #" << ( int )s.norad_id << " update on hold.";
                        goto l_end;
                    }

                    {
                    if( !s.pos_cnt_update_required.load( std::memory_order_acquire ) ) continue;

                    std::unique_lock lock{ s.pos_cnt_mtx };
                    auto result = PEARTH->_sat_update_func( s.norad_id, s.pos_cnt );
                    lock.unlock();

                    switch( result ) {
                        case EARTH_SAT_UPDATE_RESULT_OK: break;
                        case EARTH_SAT_UPDATE_RESULT_REJECT: break;
                        case EARTH_SAT_UPDATE_RESULT_RETRY: break;

                        case EARTH_SAT_UPDATE_RESULT_HOLD: {
                            s.hold_update.store( true, std::memory_order_seq_cst );
                            WARC_ECHO_RT_THAT_INTEL( PEARTH ) << "Satellite positions updater requests \"HOLD\".";
                        break; }
                    }
                    
                    }
                l_end:
                    s.pos_cnt_update_required.store( false, std::memory_order_release );
                    required_update_count.fetch_sub( 1, std::memory_order_release );
                }
            }
        }

        void join_th_update() {
            required_update_count = -1;
            required_update_count.notify_one();

            if( !hth_update.joinable() ) return; 

            hth_update.join();
        }
        
        _SATS& refresh( float elapsed ) {
            if( tick.cmpxchg_lap( 1.0 ) ) {
                for( int idx = 0; idx < 3; ++idx ) {
                    _SAT_NOAA& s = noaa[ idx ];

                    std::unique_lock lock{ s.pos_cnt_mtx, std::defer_lock_t{} };

                    if( !lock.try_lock() ) continue;

                    if( s.pos_cnt.empty() ) {
                        s.pos_cnt_update_required.store( true, std::memory_order_seq_cst );
                        required_update_count.fetch_add( 1, std::memory_order_seq_cst );
                        required_update_count.notify_one();
                    } else {
                        s.advance_pos();
                        PIMM->earth.sat_poss.get()[ idx ] = s.pos; /* ( &s - noaa ) / sizeof( _SAT_NOAA ) - always 0, why Ahri? */
                    }
                }
                PIMM->earth.sat_poss.uplink_b();
            }

            return *this;
        }

        _SATS& splash( float elapsed ) {
            for( auto& s : noaa )
                s.splash( elapsed );

            return *this;
        }

    } sats;


    struct _CTRL : Descriptor {
        IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_IMM_STR"::_CTRL")

        struct _DRAIN;
    
    #define _WARC_IMM_CTRL_PUSH_SINK( _1, _2, _3, _4 ) this->push_sink( _SINK{ name: _1, proc: _2, sexecc: _3, _csexecc: _4 } )
    #define _WARC_IMM_CTRL_SINK_PROC_NONE ( [] ( [[maybe_unused]]float ) -> int { return 0; } )
    #define _WARC_IMM_CTRL_SINK_PROC [ & ] ( float elapsed ) -> int
    #define _WARC_IMM_CTRL_SINK_SEXECC( count ) (count)
    #define _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( count ) (count)
        struct _SINK {
            std::string                     name       = nullptr;
            std::function< int( float ) >   proc       = nullptr;
            int                             sexecc     = 1;

            std::vector< _DRAIN* >          drains     = {};

            int                             _csexecc   = 0;
        };

        using SINK = size_t;

    #define _WARC_IMM_CTRL_PUSH_DRAIN( _1, _2, _3, _4 ) this->push_drain( _DRAIN{ name: _1, cond: _2, proc: _3, engd: _4 } )
    #define _WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY ( [] () -> bool { return false; } )
    #define _WARC_IMM_CTRL_DRAIN_COND_CONCISE( c ) ( [ & ] () -> bool{ return (c); } )
    #define _WARC_IMM_CTRL_DRAIN_PROC_NONE ( [] ( [[maybe_unused]]float ) -> int { return 0; } )
    #define _WARC_IMM_CTRL_DRAIN_PROC [ & ] ( float elapsed ) -> int
    #define _WARC_IMM_CTRL_DRAIN_ENGAGED( flag ) (flag)
        struct _DRAIN {
            std::string                     name     = nullptr;
            std::function< bool() >         cond     = nullptr;
            std::function< int( float ) >   proc     = nullptr;
            bool                            engd     = true;

            std::vector< _SINK* >           sinks    = {};

            bool                            _trigd   = false;
            
            void trigger() { _trigd = true; }
            void trigger( std::memory_order m  ) { std::atomic_ref< bool >{ _trigd }.store( true, m ); }
        };

        using DRAIN = size_t;

    #define _WARC_IMM_CTRL_BSD( s, d ) ( this->bind_sink_2_drain( s, d ) )
    #define _WARC_IMM_CTRL_BDS( d, s ) ( this->bind_drain_2_sink( d, s ) )
    #define _WARC_IMM_CTRL_BSDS( s1, d, s2 ) ( _WARC_IMM_CTRL_BSD( s1, d ), _WARC_IMM_CTRL_BDS( d, s2 ) )

    #define _WARC_IMM_CTRL_TOKENS( c, ... ) ( this->insert_tokens( c, __VA_ARGS__ ) )
        struct _TOKEN {
            _SINK*   sink   = nullptr;
        };

        std::vector< HVEC< _SINK > >    sinks    = {};
        std::vector< HVEC< _DRAIN > >   drains   = {}; 
        std::list< _TOKEN >             tokens   = {}; /* TO BE REPLACED with either PMR std::list or IXT's BLOCK_DIFFUSER. */


        _CTRL() {
        #pragma region CURSOR
            idxs.crs_reset = _WARC_IMM_CTRL_PUSH_SINK( "crs-reset", 
                _WARC_IMM_CTRL_SINK_PROC{
                    memset( ( void* )&crs_info, 0, sizeof( crs_info ) );
                    return 0;
                }, 
                _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 0 )
            );

            idxs.crs_frame = _WARC_IMM_CTRL_PUSH_SINK( "crs-frame",
                ( _WARC_IMM_CTRL_SINK_PROC{
                    Vec2 crs = PIMM->surf.ptr_v();
                    if( crs.x == 0.0 ) return 0;

                    float dx = crs.x - crs_info._x_last;

                    if( abs( dx ) < 0.04 ) return 0;
                    
                    if( std::signbit( dx ) == std::signbit( crs_info._dx_last ) ) goto l_end;

                    ++crs_info.x_cross_count;

                l_end:
                    crs_info._x_last = crs.x;
                    crs_info._dx_last = dx;
                    return 0;
                } ),
                _WARC_IMM_CTRL_SINK_SEXECC( -1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 0 )
            );

            idxs.crs_r2f = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "crs-r2f", _WARC_IMM_CTRL_DRAIN_COND_CONCISE( true ), _WARC_IMM_CTRL_DRAIN_PROC_NONE, _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.crs_f2r = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "crs-f2r", _WARC_IMM_CTRL_DRAIN_COND_CONCISE( false ), _WARC_IMM_CTRL_DRAIN_PROC_NONE, _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            _WARC_IMM_CTRL_BSDS( idxs.crs_reset, idxs.crs_r2f, idxs.crs_frame );
            _WARC_IMM_CTRL_BSDS( idxs.crs_frame, idxs.crs_f2r, idxs.crs_reset );
        #pragma endregion CURSOR

        #pragma region CINEMATIC
            idxs.cin_reset = _WARC_IMM_CTRL_PUSH_SINK( 
                "cin-reset", _WARC_IMM_CTRL_SINK_PROC_NONE, _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 1 ) 
            );

            idxs.cin_combo = _WARC_IMM_CTRL_PUSH_SINK(
                "cin-combo", _WARC_IMM_CTRL_SINK_PROC_NONE, _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 1 )
            );
            
            idxs.cin_r2r = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "cin-r2r", 
                _WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY,
                ( _WARC_IMM_CTRL_DRAIN_PROC{
                    switch( ++PIMM->cinematic ) {
                        case 1: PIMM->cinematic_tick.lap(); [[fallthrough]];
                        case 2: PIMM->cinematic_wy = 0.24; break;
                        default: PIMM->cinematic = 0; break;
                    }

                    return 0;
                } ), 
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.cin_r2c = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "cin-r2c", 
                _WARC_IMM_CTRL_DRAIN_COND_CONCISE( PIMM->surf.down( SurfKey::RMB ) ),
                ( _WARC_IMM_CTRL_DRAIN_PROC{
                    cin_info.trigger_count = 0;
                    cin_info.tick.lap();
                    return 0;
                } ),
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.cin_c2c = _WARC_IMM_CTRL_PUSH_DRAIN(
                "cin-c2c",
                _WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY,
                ( _WARC_IMM_CTRL_DRAIN_PROC{
                    ++cin_info.trigger_count;
                    cin_info.tick.lap();
                    return 0;
                } ),
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.cin_c2r = _WARC_IMM_CTRL_PUSH_DRAIN(
                "cin-c2r",
                _WARC_IMM_CTRL_DRAIN_COND_CONCISE( cin_info.tick.peek_lap() > 0.2 ),
                ( _WARC_IMM_CTRL_DRAIN_PROC{
                    switch( cin_info.trigger_count ) {
                        case 2: PIMM->cinematic == 1 ? PIMM->cinematic = 0 : PIMM->cinematic = 1; break;
                        case 3: PIMM->cinematic == 2 ? PIMM->cinematic = 0 : PIMM->cinematic = 2; break;

                        default: return 0;
                    }

                    PIMM->cinematic_tick.lap();
                    PIMM->cinematic_wy = 0.24;

                    return 0;
                } ),
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            _WARC_IMM_CTRL_BSDS( idxs.cin_reset, idxs.cin_r2r, idxs.cin_reset );
            _WARC_IMM_CTRL_BSDS( idxs.cin_reset, idxs.cin_r2c, idxs.cin_combo );
            _WARC_IMM_CTRL_BSDS( idxs.cin_combo, idxs.cin_c2c, idxs.cin_combo );
            _WARC_IMM_CTRL_BSDS( idxs.cin_combo, idxs.cin_c2r, idxs.cin_reset );
        #pragma endregion CINEMATIC

        #pragma region COUNTRIES
            idxs.cnt_tgl = _WARC_IMM_CTRL_PUSH_SINK( 
                "cnt-tgl", _WARC_IMM_CTRL_SINK_PROC_NONE, _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 1 )
            );

            idxs.cnt_frame = _WARC_IMM_CTRL_PUSH_SINK( 
                "cnt-frame",
                ( _WARC_IMM_CTRL_SINK_PROC{
                    if( cnt_info.triggered || !cnt_info.effective ) return 0;

                    if( cnt_info.tick.cmpxchg_lap( PEARTH_PARAMS->shake_decay ) ) {
                        cnt_info.x_cross_count_capture = std::min( cnt_info.x_cross_count_capture + 1, crs_info.x_cross_count );
                    }

                    if( crs_info.x_cross_count - cnt_info.x_cross_count_capture >= PEARTH_PARAMS->shake_cross_count ) {
                        cnt_info.triggered = true;
                        PIMM->toggle_countries();
                    }

                    return 0;
                } ),
                _WARC_IMM_CTRL_SINK_SEXECC( -1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 0 )
            );
            
            idxs.cnt_2t = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "cnt-2t", 
                _WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY, 
                ( _WARC_IMM_CTRL_DRAIN_PROC{ PIMM->toggle_countries(); return 0; } ), 
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.cnt_t2f = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "cnt-t2f", 
                _WARC_IMM_CTRL_DRAIN_COND_CONCISE( PIMM->surf.down( SurfKey::LMB ) ), 
                ( _WARC_IMM_CTRL_DRAIN_PROC{
                        cnt_info.x_cross_count_capture = crs_info.x_cross_count;
                        cnt_info.triggered = false;

                        Vec2 crs     = PIMM->surf.ptr_v() * 2.0;
                        glm::vec3 po = glm::vec3{ 0.0 } - PIMM->lens.pos;
                        glm::vec3 pt = glm::vec3{ 
                            glm::rotate( glm::mat4{ 1.0 }, glm::radians( PEARTH_PARAMS->lens_fov * crs.x ), PIMM->lens.up ) 
                            * 
                            glm::rotate( glm::mat4{ 1.0 }, glm::radians( PEARTH_PARAMS->lens_fov / PIMM->surf.aspect() * crs.y ), PIMM->lens.right() )
                            *
                            glm::vec4{ PIMM->lens.forward(), 0.0 }
                        };
                        
                        float ptn = glm::length( pt );
                        ptn *= ptn;
                        glm::vec3 pm = pt * glm::dot( po, pt ) / ptn;

                        cnt_info.effective = glm::length( pm - po ) <= 0.86;

                        return 0;
                } ), 
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.cnt_f2t = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "cnt-f2t", _WARC_IMM_CTRL_DRAIN_COND_CONCISE( !PIMM->surf.down( SurfKey::LMB ) ), _WARC_IMM_CTRL_DRAIN_PROC_NONE, _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            _WARC_IMM_CTRL_BSDS( idxs.cnt_tgl, idxs.cnt_2t, idxs.cnt_tgl );
            _WARC_IMM_CTRL_BSDS( idxs.cnt_tgl, idxs.cnt_t2f, idxs.cnt_frame );
            _WARC_IMM_CTRL_BSDS( idxs.cnt_frame, idxs.cnt_f2t, idxs.cnt_tgl );
        #pragma endregion COUNTRIES

        #pragma region SAT_HIGH
            idxs.sat_high_tgl = _WARC_IMM_CTRL_PUSH_SINK( 
                "sat-high-tgl",
                ( _WARC_IMM_CTRL_SINK_PROC{ PIMM->toggle_sat_high(); return 0; } ),
                _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 1 )
            );
            
            idxs.sat_high_2t = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "sat-high-2t", _WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY, _WARC_IMM_CTRL_DRAIN_PROC_NONE, _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            _WARC_IMM_CTRL_BSDS( idxs.sat_high_tgl, idxs.sat_high_2t, idxs.sat_high_tgl );
        #pragma endregion SAT_HIGH

        #pragma region LENS
            idxs.lens_reset = _WARC_IMM_CTRL_PUSH_SINK( 
                "lens-reset", _WARC_IMM_CTRL_SINK_PROC_NONE, _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 0 )
            );

            idxs.lens_frame = _WARC_IMM_CTRL_PUSH_SINK( 
                "lens-frame",
                ( _WARC_IMM_CTRL_SINK_PROC{
                    Vec2 cd = PIMM->surf.ptr_v();
                    if( cd.x == 0.0 ) return 0;

                    cd *= -PEARTH_PARAMS->lens_sens * PIMM->lens.l2t();

                    PIMM->lens.spin_ul( { cd.x, cd.y }, { -82.0, 82.0 } );
                    PIMM->surf.ptr_reset();
                    SurfPtr::env_to( Vec2::O() );

                    if( sat_high_info.triggered ) goto l_sat_high_end;

                    if( sat_high_info.tick.cmpxchg_lap( PEARTH_PARAMS->shake_decay ) ) {
                        sat_high_info.x_cross_count_capture = std::min( sat_high_info.x_cross_count_capture + 1, crs_info.x_cross_count );
                    }

                    if( crs_info.x_cross_count - sat_high_info.x_cross_count_capture >= PEARTH_PARAMS->shake_cross_count ) {
                        sat_high_info.triggered = true;
                        PIMM_CTRL->trigger( PIMM_CTRL->idxs.sat_high_2t, std::memory_order_relaxed );
                    }
                
                l_sat_high_end:
                    if( PIMM->cinematic != 2 ) goto l_cinematic_end;
                    PIMM->cinematic_wy = cd.x / elapsed;
                
                l_cinematic_end:
                    return 0;
                } ),
                _WARC_IMM_CTRL_SINK_SEXECC( -1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 0 )
            );

            idxs.lens_r2f = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "lens-r2f", 
                _WARC_IMM_CTRL_DRAIN_COND_CONCISE( PIMM->surf.down( SurfKey::RMB ) && PIMM->cinematic != 1 ), 
                ( _WARC_IMM_CTRL_DRAIN_PROC{
                    PIMM->surf.ptr_reset();
                    SurfPtr::env_to( Vec2::O() );

                    sat_high_info.x_cross_count_capture = crs_info.x_cross_count;
                    sat_high_info.triggered = false;

                    return 0;
                } ), 
                _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            idxs.lens_f2r = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "lens-f2r", _WARC_IMM_CTRL_DRAIN_COND_CONCISE( !PIMM->surf.down( SurfKey::RMB ) ), _WARC_IMM_CTRL_DRAIN_PROC_NONE, _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );
            
            _WARC_IMM_CTRL_BSDS( idxs.lens_reset, idxs.lens_r2f, idxs.lens_frame );
            _WARC_IMM_CTRL_BSDS( idxs.lens_frame, idxs.lens_f2r, idxs.lens_reset );
        #pragma endregion LENS

        #pragma region RENDER_MODE
            idxs.rendmode_itr = _WARC_IMM_CTRL_PUSH_SINK( 
                "rendmode-itr",
                ( [ &, ctr = 0 ] ( float elapsed ) mutable -> int {
                    switch( ++ctr ) {
                        case 0: PIMM->rend.uplink_fill(); break;
                        case 1: PIMM->rend.uplink_wireframe(); break;
                        case 2: PIMM->rend.uplink_points(); [[fallthrough]];
                        default: ctr = -1; break;
                    }

                    return 0;
                } ),
                _WARC_IMM_CTRL_SINK_SEXECC( 1 ), _WARC_IMM_CTRL_SINK_INITIAL_SEXECC( 1 )
            );

            idxs.rendmode_2t = _WARC_IMM_CTRL_PUSH_DRAIN( 
                "rendmode-2t", _WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY, _WARC_IMM_CTRL_DRAIN_PROC_NONE, _WARC_IMM_CTRL_DRAIN_ENGAGED( true )
            );

            _WARC_IMM_CTRL_BSDS( idxs.rendmode_itr, idxs.rendmode_2t, idxs.rendmode_itr );
        #pragma endregion RENDER_MODE

            PIMM->surf.socket_plug< SURFACE_EVENT_KEY >( 
                this->xtdx(), SURFACE_SOCKET_PLUG_AT_EXIT, 
            [ = ] ( SurfKey key, SURFKEY_STATE state, [[maybe_unused]]auto& ) -> void {
                switch( key ) {
                    case SurfKey::RMB: {
                        state == SURFKEY_STATE_DOWN ? PIMM->surf.hide_def_ptr() : PIMM->surf.show_def_ptr();

                        if( state != SURFKEY_STATE_DOWN ) break;
                        PIMM_CTRL->trigger( PIMM_CTRL->idxs.cin_c2c, std::memory_order_relaxed );
                    break; }

                    case SurfKey::C: {
                        if( state != SURFKEY_STATE_DOWN ) break;
                        PIMM_CTRL->trigger( PIMM_CTRL->idxs.cin_r2r, std::memory_order_relaxed );
                    break; }

                    case SurfKey::B: {
                        if( state != SURFKEY_STATE_DOWN ) break;

                        PIMM_CTRL->trigger( PIMM_CTRL->idxs.cnt_2t, std::memory_order_relaxed );
                    break; }

                    case SurfKey::SPACE: {
                        if( state != SURFKEY_STATE_DOWN ) break;

                        PIMM_CTRL->trigger( PIMM_CTRL->idxs.sat_high_2t, std::memory_order_relaxed );
                    break; }

                    case SurfKey::DOT: {
                        if( state != SURFKEY_STATE_DOWN ) break;

                        PIMM_CTRL->trigger( PIMM_CTRL->idxs.rendmode_2t, std::memory_order_relaxed );
                    break; }
                }
            } );

            PIMM->surf.socket_plug< SURFACE_EVENT_SCROLL >(
                this->xtdx(), SURFACE_SOCKET_PLUG_AT_EXIT,
                [ & ] ( Vec2 cursor, SURFSCROLL_DIRECTION dir, [[maybe_unused]]auto& ) -> void {
                    switch( dir ) {
                        case SURFSCROLL_DIRECTION_UP: PIMM->lens.zoom( .02 * PIMM->lens.l2t(), { 1.3, 8.2 } ); break;
                        case SURFSCROLL_DIRECTION_DOWN: PIMM->lens.zoom( -.02 * PIMM->lens.l2t(), { 1.3, 8.2 } ); break;
                    }
                }
            );

            _WARC_IMM_CTRL_TOKENS( true,
                idxs.crs_reset,
                idxs.cin_reset,
                idxs.cnt_tgl,
                idxs.sat_high_tgl,
                idxs.lens_reset,
                idxs.rendmode_itr
            );
        }


        struct _IDXS {
            /* Cinematic */
            SINK    cin_reset;
            DRAIN   cin_r2r;
            DRAIN   cin_r2c;
            SINK    cin_combo;
            DRAIN   cin_c2c;
            DRAIN   cin_c2r;

            /* Countries */
            SINK    cnt_tgl;
            DRAIN   cnt_2t;
            DRAIN   cnt_t2f;
            SINK    cnt_frame;
            DRAIN   cnt_f2t;

            /* Sat highlight */
            SINK    sat_high_tgl;
            DRAIN   sat_high_2t;

            /* Lens */
            SINK    lens_reset;
            DRAIN   lens_r2f;
            SINK    lens_frame;
            DRAIN   lens_f2r;

            /* Render mode */
            SINK    rendmode_itr;
            DRAIN   rendmode_2t;

            /* Cursor */
            SINK    crs_reset;
            DRAIN   crs_r2f;
            SINK    crs_frame;
            DRAIN   crs_f2r;
            
        } idxs;

        struct _CRS_INFO {
            int      x_cross_count   = 0;
            float    _x_last         = 0.0;
            float    _dx_last        = 0.0;
            Ticker   _x_cross_tick   = { ticker_lap_epoch_init_t{} };
        } crs_info;

        struct _CIN_INFO {
            int      trigger_count   = 0;
            Ticker   tick            = { ticker_lap_epoch_init_t{} };
        } cin_info;

        struct _CNT_INFO {
            int      x_cross_count_capture   = 0;
            bool     triggered               = false;
            bool     effective               = false;
            Ticker   tick                    = { ticker_lap_epoch_init_t{} };
        } cnt_info;

        struct _SAT_HIGH_INFO {
            int      x_cross_count_capture   = 0;
            bool     triggered               = false;
            Ticker   tick                    = { ticker_lap_epoch_init_t{} };
        } sat_high_info;


        template< typename S >
        size_t push_sink( S&& snk ) {
            this->sinks.emplace_back( HVEC< S >::allocc( std::forward< S >( snk ) ) );
            return this->sinks.size() - 1;
        }

        template< typename D >
        size_t push_drain( D&& dr ) {
            this->drains.emplace_back( HVEC< D >::allocc( std::forward< D >( dr ) ) );
            return this->drains.size() - 1;
        }

        template< typename ...Ts >
        void insert_tokens( bool clr, Ts... ts  ) {
            ( this->tokens.emplace_back( sinks[ ts ].get() ), ... );
        }


        void bind_sink_2_drain( size_t snk, size_t drn ) {
            this->sinks[ snk ]->drains.push_back( this->drains[ drn ].get() );
        }

        void bind_drain_2_sink( size_t drn, size_t snk ) {
            this->drains[ drn ]->sinks.push_back( this->sinks[ snk ].get() );
        }


        void trigger( size_t idx ) { this->drains[ idx ]->trigger(); }
        void trigger( size_t idx, std::memory_order m ) { this->drains[ idx ]->trigger( m ); }


        int frame( float elapsed ) {
            int status = 0;

            int step      = 1;
            int last_step = tokens.size();

            for( auto tok = tokens.begin(); step <= last_step && tok != tokens.end(); ) {
                if( ( tok->sink->sexecc < 0 ) || ( tok->sink->_csexecc + 1 <= tok->sink->sexecc ) ) {
                    status |= tok->sink->proc( elapsed );
                    ++tok->sink->_csexecc;
                } else if( tok->sink->drains.empty() ) {
                    tok = tokens.erase( tok );
                    goto l_token_end_no_itr;
                }

                for( auto& drn : tok->sink->drains ) {
                    if( 
                        !drn->engd 
                        || 
                        !( std::atomic_ref< bool >{ drn->_trigd }.load( std::memory_order_acquire ) || drn->cond() ) 
                    ) continue;

                    drn->proc( elapsed );
                    
                    /* Since drn->_trigd is read only from this thread, does it make sense to write it atomically? */
                    //std::atomic_ref< bool >{ drn->_trigd }.store( false, std::memory_order_relaxed );
                    drn->_trigd = false;

                    auto snk = drn->sinks.begin();
                    if( snk == drn->sinks.end() ) continue;

                    tok->sink->_csexecc = 0;
                    tok->sink = *snk;
                    ++snk;

                    for( ; snk != drn->sinks.end(); ++snk )
                        tokens.push_back( _TOKEN{ sink: *snk } );
                }

            l_token_end:
                ++tok;
            l_token_end_no_itr:
                ++step;
            }

            return status;
        }

    } ctrl;


    _IMM& refresh( float elapsed ) {
        if( cinematic == 1 ) {
            lens.spin_ul( { cinematic_wy * elapsed, cos( cinematic_tick.peek_lap() / 2.2 ) * 0.001 }, { -82.0, 82.0 } );
        } else if( cinematic == 2 ) {
            lens.spin_ul( { cinematic_wy * elapsed, 0.0 }, { -82.0, 82.0 } );
        }


        sun.refresh( elapsed );
        sats.refresh( elapsed );

        for( int idx = 0; idx < 3; ++idx ) {
            auto& s = sats.noaa[ idx ];

            ggfloat_t high_fac = 0.2 + ( 1.0 + glm::pow( sin( sats.tick.up_time() * 8.6 + s.norad_id % 10 ), 3.0 ) );

            s.high_spec.uplink_bv(
                s.base_high_spec
                *
                glm::vec3{ high_fac }
                *
                glm::vec3{ 1.0f, ( float )!s.pos_cnt.empty(), ( float )!s.pos_cnt.empty() }
            );
            
            earth.sat_high_specs.get()[ idx ] = s.high_spec.get();
        }
        
        earth.sat_high_specs.uplink_b();
        earth.countries.uplink_b();

        ufrm.sat_high.uplink_b();

        ufrm.lens_pos.uplink_bv( lens.pos );
        ufrm.view.uplink_bv( lens.view() );

        ufrm.rtc.uplink_bv( ufrm.rtc.get() + elapsed );
        
        return *this;
    }

    _IMM& splash( float elapsed ) {            
        rend.downlink_face_culling();
        galaxy.mesh.splash();
        rend.uplink_face_culling();

        earth.mesh.splash();
        sats.splash( elapsed );

        return *this;
    }
    

    void toggle_sat_high() {
        ufrm.sat_high.get() = 1.0 - ufrm.sat_high.get();
    }

    void toggle_countries() {
        earth.countries.get() ^= 1;
    }

};


void EARTH::set_sat_pos_update_func( SatUpdateFunc func ) {
    _sat_update_func = func;
}

void EARTH::sat_pos_update_hold_resume() {
    //PIMM->sats.hold_update.store( false, std::memory_order_relaxed );
}


void EARTH::set_params( IXT::SPtr< EARTH_PARAMS > ptr ) {
    _params = std::move( ptr );
}

EARTH_PARAMS& EARTH::params() {
    return *_params;
}


int EARTH::main( int argc, char* argv[] ) {
    _impl_earth = this;

    _IMM imm;

    Ticker tick;
   
    while( !imm.surf.down( SurfKey::ESC ) ) {
        float elapsed = tick.lap();
        
        imm.rend.clear( glm::vec4{ 0.0, 0.0, 0.0, 1.0 } );
        imm.ctrl.frame( elapsed );
        imm.refresh( elapsed ).splash( elapsed );
        imm.rend.swap();
    }

    imm.sats.join_th_update();
    imm.surf.downlink();

    _impl_earth = nullptr;

    return 0;
}


} };
