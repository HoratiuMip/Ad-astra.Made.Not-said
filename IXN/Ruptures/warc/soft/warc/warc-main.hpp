#pragma once

#include <warc/common.hpp>
#include <warc/inet-tls.hpp>
#include <warc/satellite.hpp>
#include <warc/n2yo.hpp>
#include <warc/earth-imm.hpp>
#include <warc/astro.hpp>
#include <warc/database.hpp>

#include <IXN/descriptor.hpp>
#include <IXN/file-manip.hpp>
#include <IXN/init.hpp>

#include <boost/json.hpp>

namespace warc {


#define WARC_MAIN_STR WARC_STR""

#define WARC_MAIN_PARSE_PROC_FUNC( func_name ) int func_name( int argc, char* argv[], const char* process )


class MAIN : ixN::Descriptor {
public:
    IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( WARC_MAIN_STR );

_WARC_PROTECTED:
    friend struct _INTERNAL;

_WARC_PROTECTED:
    ixN::DWORD                 _ixt_init_flags   = 0;

    n2yo::_N2YO                _n2yo             = {};
    ixN::SPtr< imm::EARTH >    _earth            = nullptr;   

    bool                       _database         = false;

_WARC_PROTECTED:
    int _parse_opts( int argc, char* argv[] );

    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_root_dir );

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
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_earth_imm_lens_fov );
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_earth_imm_shake_decay );
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_earth_imm_shake_cross_count );


    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_astro_ref_vernal_equinox_ts );
    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_astro_ref_first_january_ts );

    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_dev_barruncuda_controller );

    WARC_MAIN_PARSE_PROC_FUNC( _parse_proc_database );

public:
    int main( int argc, char* argv[] );

public:
    imm::EARTH& imm_earth() { return *_earth; }

};


};
