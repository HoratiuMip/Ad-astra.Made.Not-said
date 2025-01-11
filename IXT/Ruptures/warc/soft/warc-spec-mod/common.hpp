#pragma once

#include <warc/common.hpp>

#include <IXT/hyper-vector.hpp>

namespace warc { namespace spec_mod {


#define WARC_SPEC_MOD_STR WARC_STR"::spec_mod"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_SPEC_MOD_STR );


typedef   const char*   device_name_t;


class DEVICE {
_WARC_PROTECTED:
    //IXT::HVEC< void >   _hard_params   = nullptr;
    void*               _soft_params   = nullptr;

public:
    template< typename T >
    T* soft_params() { 
        return ( T* )_soft_params; 
    }

    template< typename T >
    int set_soft_params( T* params ) {
        _soft_params = ( void* )params;
        return 0;
    }

};

int push_device( device_name_t name, IXT::HVEC< DEVICE > device );
std::optional< IXT::HVEC< DEVICE > > pull_device( device_name_t name );
IXT::HVEC< DEVICE > extract_device( device_name_t name );
int purge_device( device_name_t name );


} };