#pragma once

#include <warc/common.hpp>

#include <IXT/render3.hpp>
#include <IXT/tempo.hpp>

#include <ekg/ekg.hpp>

namespace warc { namespace imm {


#define WARC_IMM_STR WARC_STR"::imm"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_IMM_STR );


class EARTH : public IXT::Descriptor {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_IMM_STR"::EARTH" );

_WARC_PROTECTED:

public:
    int main( int argc, char* argv[] );

};


} };