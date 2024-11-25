#pragma once

#include <warc/common.hpp>
#include <warc/satellite.hpp>

#include <IXT/render3.hpp>
#include <IXT/tempo.hpp>

namespace warc { namespace imm {


#define WARC_IMM_STR WARC_STR"::imm"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_IMM_STR );


class EARTH : public IXT::Descriptor {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_IMM_STR"::EARTH" );

public:
    using SatPosUpdateFunc = std::function< int( sat::NORAD_ID, std::deque< sat::POSITION >&, int ) >;

_WARC_PROTECTED:
    SatPosUpdateFunc   _sat_update_func   = nullptr;

public:
    int main( int argc, char* argv[] );

    void set_sat_pos_update_func( SatPosUpdateFunc func ) {
        _sat_update_func = func;
    }

};


} };