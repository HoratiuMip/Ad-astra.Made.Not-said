#include <warc-spec-mod/common.hpp>

namespace warc { namespace spec_mod {


static struct _INTERNAL {
    std::mutex                                     devices_mtx;
    std::map< std::string, NLN::HVEC< DEVICE > >   devices;

} _internal;

int push_device( device_name_t name, NLN::HVEC< DEVICE > device ) {
    std::unique_lock lock{ _internal.devices_mtx };

    _internal.devices[ name ].vector( std::move( device ) );
    return 0;
}

std::optional< NLN::HVEC< DEVICE > > pull_device( device_name_t name ) {
    std::unique_lock lock{ _internal.devices_mtx };
    auto itr = _internal.devices.find( name );
    lock.unlock();

    if( itr == _internal.devices.end() ) return {};
    return itr->second;
}

NLN::HVEC< DEVICE >* deep_pull_device( device_name_t name ) {
    std::unique_lock lock{ _internal.devices_mtx };
    auto itr = _internal.devices.find( name );
    lock.unlock();

    return itr == _internal.devices.end() ? nullptr : &itr->second;
}

NLN::HVEC< DEVICE > extract_device( device_name_t name ) {
    NLN::HVEC< DEVICE > ret = NULL;

    std::unique_lock lock{ _internal.devices_mtx };

    auto itr = _internal.devices.find( name );
    if( itr == _internal.devices.end() ) return NULL;

    ret.vector( std::move( itr->second ) );

    _internal.devices.erase( itr );
    return 0;
}

int purge_device( device_name_t name ) {
    std::unique_lock lock{ _internal.devices_mtx };

    _internal.devices.erase( name );
    return 0;
}

static int _trigger_devices( bool set, warc::MAIN& main, NLN_COMMS_ECHO_ARG ) {
    int status = 0;

    std::unique_lock lock{ _internal.devices_mtx };
    for( auto& [ name, device ] : _internal.devices ) {
        WARC_ECHO_RT_INTEL << ( set ? "Setting" : "Engaging" ) << " device \"" << name << "\".";
       
        if( int dev_status = ( set ? device->set( main, echo ) : device->engage( main, echo ) ); dev_status != 0 ) {
            WARC_ECHO_RT_WARNING << "Could not " << ( set ? "set" : "engage" ) << " device \"" << name << "\" properly.";
            status |= dev_status;
        } else {
            WARC_ECHO_RT_OK << ( set ? "Set" : "Engaged" ) << " device \"" << name << "\".";
        }
    }

    return status;
}

int set_devices( warc::MAIN& main, NLN_COMMS_ECHO_NO_DFT_ARG ) {
    return _trigger_devices( true, main, echo );
}

int engage_devices( warc::MAIN& main, NLN_COMMS_ECHO_NO_DFT_ARG ) {
    return _trigger_devices( false, main, echo );
}

int disengage_devices( std::memory_order mo, warc::MAIN& main, NLN_COMMS_ECHO_NO_DFT_ARG ) {
    int status = 0;

    std::unique_lock lock{ _internal.devices_mtx };
    for( auto& [ name, device ] : _internal.devices ) {
        WARC_ECHO_RT_INTEL << "Disengaging device \"" << name << "\".";

        if( int dev_status = device->request_disengage( mo, main, echo ); dev_status != 0 ) {
            WARC_ECHO_RT_WARNING << "Could not disengage device \"" << name << "\" properly.";
            status |= dev_status;
        } else {
            WARC_ECHO_RT_OK << "Disengaged device \"" << name << "\".";
        }
    }

    return status;
}

} };