#pragma once

#include <warc/common.hpp>
#include <warc/satellite.hpp>
#include <warc/astro.hpp>

#include <IXT/render3.hpp>
#include <IXT/tempo.hpp>

namespace warc { namespace imm {


#define WARC_IMM_STR WARC_STR"::imm"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_IMM_STR );


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

class EARTH : public IXT::Descriptor {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_IMM_STR"::EARTH" );

public:
    using SatUpdateFunc = std::function< EARTH_SAT_UPDATE_RESULT( sat::NORAD_ID, std::deque< sat::POSITION >& ) >;

_WARC_PROTECTED:
    friend struct _IMM;

_WARC_PROTECTED:
    SatUpdateFunc               _sat_update_func   = nullptr;
    IXT::SPtr< EARTH_PARAMS >   _params            = { std::make_shared< EARTH_PARAMS >() }; 

public:
    int main( int argc, char* argv[] );

    void set_sat_pos_update_func( SatUpdateFunc func );
    void sat_pos_update_hold_resume();

    void set_params( IXT::SPtr< EARTH_PARAMS > ptr );
    EARTH_PARAMS& params();

};


} };