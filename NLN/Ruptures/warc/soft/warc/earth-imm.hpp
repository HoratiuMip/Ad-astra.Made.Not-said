#pragma once

#include <warc/common.hpp>
#include <warc/satellite.hpp>
#include <warc/astro.hpp>

#include <NLN/render3.hpp>
#include <NLN/tempo.hpp>

namespace warc { namespace imm {


#define WARC_IMM_STR WARC_STR"::imm"
_WARC_NLN_COMPONENT_DESCRIPTOR( WARC_IMM_STR );


enum EARTH_SAT_UPDATE_RESULT : int {
    EARTH_SAT_UPDATE_RESULT_OK,
    EARTH_SAT_UPDATE_RESULT_REJECT,
    EARTH_SAT_UPDATE_RESULT_RETRY,
    EARTH_SAT_UPDATE_RESULT_HOLD,

    _EARTH_SAT_UPDATE_RESULT_FORCE_DWORD = 0x7f'ff'ff'ff
};

struct EARTH_PARAMS {
    float   lens_sens           = 1.0;
    float   lens_fov            = 60.0;
    float   shake_decay         = 0.26;
    int     shake_cross_count   = 4;
};

struct EARTH_CTRL_PARAMS {
    struct _DRAIN;
    
#define _WARC_IMM_CTRL_PUSH_SINK( _1, _2, _3, _4 ) this->push_sink( _SINK{ name: _1, proc: _2, sexecc: _3, _csexecc: _4 } )
#define WARC_IMM_CTRL_SINK_PROC_NONE ( [] ( [[maybe_unused]]float ) -> int { return 0; } )
#define WARC_IMM_CTRL_SINK_PROC [ & ] ( float elapsed ) -> int
#define WARC_IMM_CTRL_SINK_SEXECC( count ) (count)
#define WARC_IMM_CTRL_SINK_INITIAL_SEXECC( count ) (count)
    struct _SINK {
        std::string                     name       = {};
        std::function< int( float ) >   proc       = nullptr;
        int                             sexecc     = 1;

        std::vector< _DRAIN* >          drains     = {};

        int                             _csexecc   = 0;
    };

#define _WARC_IMM_CTRL_PUSH_DRAIN( _1, _2, _3, _4 ) this->push_drain( _DRAIN{ name: _1, cond: _2, proc: _3, engd: _4 } )
#define WARC_IMM_CTRL_DRAIN_COND_TRIGGER_ONLY ( [] () -> bool { return false; } )
#define WARC_IMM_CTRL_DRAIN_COND_CONCISE( c ) ( [ & ] () -> bool{ return (c); } )
#define WARC_IMM_CTRL_DRAIN_PROC_NONE ( [] ( [[maybe_unused]]float ) -> int { return 0; } )
#define WARC_IMM_CTRL_DRAIN_PROC [ & ] ( float elapsed ) -> int
#define WARC_IMM_CTRL_DRAIN_ENGAGED( flag ) ( flag )
    struct _DRAIN {
        std::string                     name     = {};
        std::function< bool() >         cond     = nullptr;
        std::function< int( float ) >   proc     = nullptr;
        bool                            engd     = true;

        std::vector< _SINK* >           sinks    = {};

        bool                            _trigd   = false;
        
        void trigger() { _trigd = true; }
        void trigger( std::memory_order m  ) { std::atomic_ref< bool >{ _trigd }.store( true, m ); }
    };

#define _WARC_IMM_CTRL_BSD( s, d ) ( this->bind_sink_2_drain( s, d ) )
#define _WARC_IMM_CTRL_BDS( d, s ) ( this->bind_drain_2_sink( d, s ) )
#define _WARC_IMM_CTRL_BSDS( s1, d, s2 ) ( _WARC_IMM_CTRL_BSD( s1, d ), _WARC_IMM_CTRL_BDS( d, s2 ) )

#define _WARC_IMM_CTRL_TOKENS( c, ... ) ( this->insert_tokens( c, __VA_ARGS__ ) )
#define WARC_IMM_CTRL_INSERT_TOKENS_NO_CLEAR ( 0 )
#define WARC_IMM_CTRL_INSERT_TOKENS_CLEAR ( 1 )
    struct _TOKEN {
        _SINK*   sink   = nullptr;
    };

    std::vector< NLN::HVEC< _SINK > >    sinks    = {};
    std::vector< NLN::HVEC< _DRAIN > >   drains   = {}; 
    std::list< _TOKEN >                  tokens   = {}; /* TO BE REPLACED with either PMR std::list or NLN's BLOCK_DIFFUSER. */

    
    template< typename S >
    size_t push_sink( S&& snk ) {
        this->sinks.emplace_back( NLN::HVEC< S >::allocc( std::forward< S >( snk ) ) );
        return this->sinks.size() - 1;
    }

    template< typename D >
    size_t push_drain( D&& dr ) {
        this->drains.emplace_back( NLN::HVEC< D >::allocc( std::forward< D >( dr ) ) );
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


    using SINK  = size_t;
    using DRAIN = size_t;

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
        int           x_cross_count   = 0;
        float         _x_last         = 0.0;
        float         _dx_last        = 0.0;
        NLN::Ticker   _x_cross_tick   = { NLN::ticker_lap_epoch_init_t{} };
    } crs_info;

    struct _CIN_INFO {
        int           trigger_count   = 0;
        NLN::Ticker   tick            = { NLN::ticker_lap_epoch_init_t{} };
    } cin_info;

    struct _CNT_INFO {
        int           x_cross_count_capture   = 0;
        bool          triggered               = false;
        bool          effective               = false;
        NLN::Ticker   tick                    = { NLN::ticker_lap_epoch_init_t{} };
    } cnt_info;

    struct _SAT_HIGH_INFO {
        int           x_cross_count_capture   = 0;
        bool          triggered               = false;
        NLN::Ticker   tick                    = { NLN::ticker_lap_epoch_init_t{} };
    } sat_high_info;
};

class EARTH : public NLN::Descriptor {
public:
    NLN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_IMM_STR"::EARTH" );

public:
    using on_ready_callback_t = std::function< int() >;
    using SatUpdateFunc = std::function< EARTH_SAT_UPDATE_RESULT( sat::NORAD_ID, std::deque< sat::POSITION >& ) >;

_WARC_PROTECTED:
    friend struct _IMM;

_WARC_PROTECTED:
    SatUpdateFunc               _sat_update_func   = nullptr;
    NLN::SPtr< EARTH_PARAMS >   _params            = { std::make_shared< EARTH_PARAMS >() }; 

public:
    int main( int argc, char* argv[], on_ready_callback_t on_ready_callback );

    void set_sat_pos_update_func( SatUpdateFunc func );
    void sat_pos_update_hold_resume();

    void set_params( NLN::SPtr< EARTH_PARAMS > ptr );
    EARTH_PARAMS& params();

    EARTH_CTRL_PARAMS& ctrl();
    void lens_spin( glm::vec2 thetas );
    void lens_zoom( float delta );

};


} };