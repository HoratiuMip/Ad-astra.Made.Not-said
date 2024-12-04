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


#define WARC_MAIN_STR WARC_STR""

#define WARC_MAIN_PARSE_PROC_FUNC( func_name ) int func_name( int argc, char* argv[], const char* process )


class MAIN : IXT::Descriptor {
public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_MAIN_STR );

_WARC_PROTECTED:
    friend struct _INTERNAL;

_WARC_PROTECTED:
    n2yo::_N2YO                _n2yo    = {};
    IXT::SPtr< imm::EARTH >    _earth   = nullptr;   

_WARC_PROTECTED:
    int _parse_opts( int argc, char* argv[] );

    /*
    | Usage:
    | --from-config <../some_config_file.json>
    */
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_from_config );

    /*
    | Usage:
    | --n2yo-api-key use <XXX>
    | --n2yo-api-key burn <XXX>
    | --n2yo-api-key use ash
    | --n2yo-api-key show ash
    */
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_n2yo_api_key );

    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_n2yo_ip );

    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_n2yo_bulk_count );

    /*
    | Usage:
    | --n2yo-bypass
    */
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_n2yo_mode );

    /*
    | Usage:
    | --earth-imm
    */
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_earth_imm );

    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_earth_imm_lens_sens );

public:
    int main( int argc, char* argv[] );
};


};
