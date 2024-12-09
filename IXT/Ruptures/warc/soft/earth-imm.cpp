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

#define ELAPSED_ARGS_DECL double ela, double rela
#define ELAPSED_ARGS_CALL ela, rela


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
    bool        cinematic        = false;
    Ticker      cinematic_tick   = {};

    struct _UFRM {
        _UFRM()
        : view{ "view", PIMM->lens.view() },
            proj{ "proj", glm::perspective( glm::radians( 55.0f ), PIMM->surf.aspect(), 0.1f, 1000.0f ) },
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

        _SUN& refresh( ELAPSED_ARGS_DECL ) {
            if( tick.cmpxchg_lap( 30.0 ) ) {
                this->_load_rt_pos();
            }

            return *this;
        }

    } sun;

    struct _EARTH {
        _EARTH() 
        : mesh{ WARC_RUPTURE_IMM_ROOT_DIR"earth/", "earth", MESH3_FLAG_MAKE_SHADING_PIPE },
            sat_poss{ "sat_poss" },
            sat_high_specs{ "sat_high_specs" },
            show_countries{ "show_countries", 0 }
        {
            mesh.model.uplink_v( glm::rotate( glm::mat4{ 1.0 }, -PIf / 2.0f, glm::vec3{ 0, 1, 0 } ) * mesh.model.get() );

            mesh.pipe->pull( 
                PIMM_UFRM->view, PIMM_UFRM->proj,
                PIMM_UFRM->lens_pos, 
                PIMM_UFRM->rtc,
                PIMM_UFRM->sat_high,
                PIMM->sun.pos,
                this->sat_poss, this->sat_high_specs, this->show_countries
            );

            PIMM->sun.pos.uplink_b();
        }

        Mesh3                        mesh;
        Uniform3< glm::vec3[ 3 ] >   sat_poss;
        Uniform3< glm::vec3[ 3 ] >   sat_high_specs;
        Uniform3< glm::i32 >         show_countries;

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

            for( auto& s : noaa ) {
                s.mesh.pipe->pull( PIMM_UFRM->sat_high );
            }
        }

        Ticker                  tick;
        std::thread             hth_update;
        std::atomic< int >      required_update_count   = 0;

        struct _SAT_NOAA {
            static constexpr float BASE_ELEVATION = 1.086;

            _SAT_NOAA( sat::NORAD_ID nid )
            : norad_id{ nid },
                mesh{ WARC_RUPTURE_IMM_ROOT_DIR"sat_noaa/", "sat_noaa", MESH3_FLAG_MAKE_SHADING_PIPE },
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
            std::atomic< int >            pos_cnt_update_required   = false;
            std::atomic< bool >           hold_update               = false;

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
        
        _SATS& refresh( ELAPSED_ARGS_DECL ) {
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

        _SATS& splash( ELAPSED_ARGS_DECL ) {
            for( auto& s : noaa )
                s.splash( ELAPSED_ARGS_CALL );

            return *this;
        }

    } sats;


    struct _CTRL : Descriptor {
        IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_IMM_STR"::_CTRL")

        struct _DRAIN;
    
    #define _WARC_IMM_CTRL_PUSH_SINK( _1, _2, _3 ) this->push_sink( _SINK{ name: _1, proc: _2, iec: _3 } )
        struct _SINK {
            const char*              name     = nullptr;
            std::function< int() >   proc     = nullptr;
            int                      iec      = 1;

            std::vector< _DRAIN* >   drains   = {};

            int                      _cec     = 0;
        };

    #define _WARC_IMM_CTRL_PUSH_DRAIN( _1, _2, _3, _4 ) this->push_drain( _DRAIN{ name: _1, cond: _2, fire: _3, engd: _4 } )
        struct _DRAIN {
            const char*               name    = nullptr;
            std::function< bool() >   cond    = nullptr;
            bool                      fire    = false;
            bool                      engd    = true;

            std::vector< _SINK* >     sinks   = {};
        };

    #define _WARC_IMM_CTRL_BSD( s, d ) ( this->bind_sink_2_drain( s, d ) )
    #define _WARC_IMM_CTRL_BDS( d, s ) ( this->bind_drain_2_sink( d, s ) )
    #define _WARC_IMM_CTRL_BSDS( s1, d, s2 ) ( _WARC_IMM_CTRL_BSD( s1, d ), _WARC_IMM_CTRL_BDS( d, s2 ) )

    #define _WARC_IMM_CTRL_TOKENS( c, ... ) ( this->insert_tokens( c, __VA_ARGS__ ) )
        struct _TOKEN {
            _SINK*   sink   = nullptr;
        };

        std::vector< IXT::SPtr< _SINK > >    sinks    = {};
        std::vector< IXT::SPtr< _DRAIN > >   drains   = {}; 
        std::list< _TOKEN >                  tokens   = {};

        _CTRL() {
        #pragma region TEST
            {
            _WARC_IMM_CTRL_PUSH_SINK( "Control test sink 1.",
                ( [ & ] () -> int { WARC_ECHO_RT_DEBUG << sinks[ 0 ]->name; return 0; } ),
                1
            );

            _WARC_IMM_CTRL_PUSH_SINK( "Control test sink 2.",
                ( [ & ] () -> int { WARC_ECHO_RT_DEBUG << sinks[ 1 ]->name; return 0; } ),
                3
            );

            _WARC_IMM_CTRL_PUSH_DRAIN( "Control drain test sink 1 to 2, activate on key F1.",
                ( [ & ] () -> bool { return PIMM->surf.down( SurfKey::F1 ); } ),
                false, true
            );

            _WARC_IMM_CTRL_PUSH_DRAIN( "Control drain test sink 2 to 1, activate on key F2.",
                ( [ & ] () -> bool { return PIMM->surf.down( SurfKey::F2 ); } ),
                false, true
            );

            _WARC_IMM_CTRL_BSDS( 0, 0, 1 );
            _WARC_IMM_CTRL_BSDS( 1, 1, 0 );

            _WARC_IMM_CTRL_TOKENS( false, 0 );
            }
        #pragma endregion TEST

        #pragma region LENS
            {
            static struct _SHAKE_SAT_HIGH {
                int      cross_count   = 0;
                float    last_x        = 0;
                bool     triggered     = false;
                Ticker   tick;

            } shake_sat_high;

            auto idle = _WARC_IMM_CTRL_PUSH_SINK( "lens-idle",
                ( [ & ] () -> int { 
                    PIMM->surf.show_def_ptr();
                    
                    return 0; 
                } ),
                1
            );

            auto init = _WARC_IMM_CTRL_PUSH_SINK( "lens-init",
                ( [ & ] () -> int {
                    static Ticker tick{ ticker_lap_epoch_init_t{} };

                    PIMM->surf.hide_def_ptr();

                    if( tick.lap() <= 0.2 )
                        PIMM->toggle_cinematic();

                    PIMM->surf.ptr_reset();
                    SurfPtr::env_to( Vec2::O() );

                    shake_sat_high.cross_count = 0;
                    shake_sat_high.triggered   = false;

                    return 0;
                } ),
                1
            );

            auto loop = _WARC_IMM_CTRL_PUSH_SINK( "lens-loop",
                ( [ & ] () -> int {
                    Vec2 cd = PIMM->surf.ptr_v();
                    if( cd.x == 0.0 ) return 0;

                    cd *= -PEARTH_PARAMS->lens_sens * PIMM->lens.l2t();

                    PIMM->lens.spin_ul( { cd.x, cd.y }, { -82.0, 82.0 } );
                    PIMM->surf.ptr_reset();
                    SurfPtr::env_to( Vec2::O() );

                    if( shake_sat_high.triggered ) return 0;

                    if( shake_sat_high.tick.cmpxchg_lap( PEARTH_PARAMS->sat_high_decay ) ) {
                        shake_sat_high.cross_count = std::max( shake_sat_high.cross_count - 1, 0 );
                    }
                    
                    if( std::signbit( cd.x ) == std::signbit( shake_sat_high.last_x ) ) return 0;

                    if( ++shake_sat_high.cross_count >= PEARTH_PARAMS->sat_high_cross ) {
                        shake_sat_high.cross_count = 0;

                        shake_sat_high.triggered = true;
                        PIMM->toggle_sat_high();
                    }

                    shake_sat_high.last_x = cd.x;

                    return 0;
                } ),
                -1
            );

            auto idle2init = _WARC_IMM_CTRL_PUSH_DRAIN( "lens-idle-to-init",
                ( [ & ] () -> bool { return PIMM->surf.down( SurfKey::RMB ); } ),
                false, true
            );

            auto init2loop = _WARC_IMM_CTRL_PUSH_DRAIN( "lens-init-to-loop",
                ( [ & ] () -> bool { return true; } ),
                false, true
            );

            auto loop2idle = _WARC_IMM_CTRL_PUSH_DRAIN( "lens-loop-to-idle",
                ( [ & ] () -> bool { return !PIMM->surf.down( SurfKey::RMB ); } ),
                false, true
            );
            
            _WARC_IMM_CTRL_BSDS( idle, idle2init, init );
            _WARC_IMM_CTRL_BSDS( init, init2loop, loop );
            _WARC_IMM_CTRL_BSDS( loop, loop2idle, idle );

            _WARC_IMM_CTRL_TOKENS( false, idle );
            }
        #pragma endregion LENS

        }


        template< typename S >
        size_t push_sink( S&& snk ) {
            this->sinks.emplace_back( std::make_shared< S >( std::forward< S >( snk ) ) );
            return this->sinks.size() - 1;
        }

        template< typename D >
        size_t push_drain( D&& dr ) {
            this->drains.emplace_back( std::make_shared< D >( std::forward< D >( dr ) ) );
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

    
        int frame() {
            int status = 0;

            int step      = 1;
            int last_step = tokens.size();

            for( auto tok = tokens.begin(); step <= last_step && tok != tokens.end(); ++tok ) {
                bool drained = false;

                if( ++tok->sink->_cec <= tok->sink->iec || tok->sink->iec < 0 ) {
                    status |= tok->sink->proc();
                } else if( tok->sink->drains.empty() ) {
                    drained = true;
                }

                for( auto& drn : tok->sink->drains ) {
                    if( !drn->engd || !( drn->fire || drn->cond() ) ) goto l_token_end;

                    drained = true;
                    for( auto& snk : drn->sinks )
                        tokens.push_back( _TOKEN{ sink: snk } );
                }

                if( drained ) {
                    tok->sink->_cec = 0;
                    tok = tokens.erase( tok );
                }

            l_token_end:
                ++step;
            }

            return status;
        }

    } ctrl;


    _IMM& control( ELAPSED_ARGS_DECL ) {
        static Ticker tick;
        static int    configd_control_scheme = 0; 

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

                    surf.on< SURFACE_EVENT_KEY >( [ & ] ( SurfKey key, SURFKEY_STATE state, [[maybe_unused]]auto& ) -> void {
                        switch( key ) {
                            case SurfKey::SPACE: {
                                if( state != SURFKEY_STATE_UP ) break;
                                this->toggle_sat_high();
                            break; }

                            case SurfKey::C: {
                                if( state != SURFKEY_STATE_UP ) break;
                                this->toggle_cinematic();
                            break; }

                            case SurfKey::B: {
                                if( state != SURFKEY_STATE_UP ) break;
                                this->toggle_show_countries();
                            break; }

                        }
                    } );   
                    
                    surf.on< SURFACE_EVENT_POINTER >( [ & ] ( Vec2 cursor, Vec2 prev_cursor, [[maybe_unused]]auto& ) -> void {
                        
                    } );

                break; }
            }
            configd_control_scheme = control_scheme;
        }


        if( surf.down( SurfKey::COMMA ) )
            rend.uplink_wireframe();
        if( surf.down( SurfKey::DOT ) )
            rend.downlink_wireframe();
        

        ctrl.frame();


        return *this;
    }

    _IMM& refresh( ELAPSED_ARGS_DECL ) {
        ggfloat_t high_fac = 0.2 + ( 1.0 + glm::pow( sin( sats.tick.up_time() * 8.6 ), 3.0 ) );


        if( cinematic ) {
            lens.spin_ul( { 0.004 * ela, cos( cinematic_tick.peek_lap() / 2.2 ) * 0.001 }, { -82.0, 82.0 } );
        }


        sun.refresh( ELAPSED_ARGS_CALL );
        sats.refresh( ELAPSED_ARGS_CALL );

        for( int idx = 0; idx < 3; ++idx ) {
            auto& s = sats.noaa[ idx ];

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
        earth.show_countries.uplink_b();

        ufrm.sat_high.uplink_b();

        ufrm.lens_pos.uplink_bv( lens.pos );
        ufrm.view.uplink_bv( lens.view() );

        ufrm.rtc.uplink_bv( ufrm.rtc.get() + rela );
        
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
    

    void toggle_sat_high() {
        ufrm.sat_high.get() = 1.0 - ufrm.sat_high.get();
    }

    void toggle_show_countries() {
        earth.show_countries.get() ^= 1;
    }

    void toggle_cinematic() {
        cinematic ^= true;
        cinematic_tick.lap();
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
        double elapsed_raw = tick.lap();
        double elapsed = elapsed_raw * 60.0;
        
        imm.rend.clear( glm::vec4{ 0.1, 0.1, 0.1, 1.0 } );
        imm.control( elapsed, elapsed_raw ).refresh( elapsed, elapsed_raw ).splash( elapsed, elapsed_raw );
        imm.rend.swap();
    }

    imm.sats.join_th_update();
    imm.surf.downlink();

    _impl_earth = nullptr;

    return 0;
}


} };
