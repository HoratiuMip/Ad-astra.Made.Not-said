#pragma once

#include <warc/common.hpp>
#include <warc/inet-tls.hpp>
#include <warc/satellite.hpp>
#include <warc/n2yo.hpp>
#include <warc/earth-imm.hpp>

#include <IXT/descriptor.hpp>
#include <IXT/file-manip.hpp>
#include <IXT/init.hpp>

#include <boost/json.hpp>

namespace warc {


#define WARC_MAIN_STR WARC_STR"::MAIN"


class MAIN : IXT::Descriptor {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_MAIN_STR );

_WARC_PROTECTED:
    n2yo::_N2YO                _n2yo    = {};
    IXT::SPtr< imm::EARTH >    _earth   = nullptr;   

_WARC_PROTECTED:
    int _parse_opts( int argc, char* argv[] );

    /*
    | Usage:
    | --n2yo-api-key use XXX
    | --n2yo-api-key burn XXX
    | --n2yo-api-key use ash
    | --n2yo-api-key show ash
    */
    int _parse_proc_n2yo_api_key( char* argv[], const char* process );

    /*
    | Usage:
    | --earth-imm
    */
    int _parse_proc_earth_imm( char* argv[], const char* process );

public:
    int main( int argc, char* argv[], VOID_DOUBLE_LINK vdl );
};


};
