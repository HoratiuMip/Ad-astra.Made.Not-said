#pragma once

#include <warc/common.hpp>
#include <warc/warc-main.hpp>

#include <IXN/hyper-vector.hpp>

namespace warc { namespace dev {


#define WARC_DEV_STR WARC_STR"::dev"
_WARC_IXN_COMPONENT_DESCRIPTOR( WARC_DEV_STR );


typedef   const char*   device_name_t;


class DEVICE {
public:
    DEVICE() = default;

    virtual ~DEVICE() {}

_WARC_PROTECTED:
    std::atomic< bool >   _engaged      = true;

    //ixN::HVEC< void >   _hard_params   = nullptr;
    void*                _soft_params   = nullptr;

public:
    virtual int set( warc::MAIN& main, IXN_COMMS_ECHO_ARG ) = 0;
    virtual int engage( warc::MAIN& main, IXN_COMMS_ECHO_ARG ) = 0;
    virtual int disengage( warc::MAIN& main, IXN_COMMS_ECHO_ARG ) = 0;

    int request_disengage( std::memory_order mo, warc::MAIN& main, IXN_COMMS_ECHO_ARG ) { 
        _engaged.store( false, mo );  
        return this->disengage( main, echo );
    }

public:
    template< typename T > T* soft_params() { 
        return ( T* )_soft_params; 
    }

    template< typename T > int set_soft_params( T* params ) {
        _soft_params = ( void* )params;
        return 0;
    }

};

int push_device( device_name_t name, ixN::HVEC< DEVICE > device );

std::optional< ixN::HVEC< DEVICE > > pull_device( device_name_t name );
ixN::HVEC< DEVICE >* deep_pull_device( device_name_t name );

ixN::HVEC< DEVICE > extract_device( device_name_t name );

int purge_device( device_name_t name );

int set_devices( warc::MAIN& main, IXN_COMMS_ECHO_ARG );
int engage_devices( warc::MAIN& main, IXN_COMMS_ECHO_ARG );
int disengage_devices( std::memory_order mo, warc::MAIN& main, IXN_COMMS_ECHO_ARG );


} };