#include <warc-spec-mod/common.hpp>

namespace warc { namespace spec_mod {


static struct _INTERNAL {
    std::mutex                                     devices_mtx;
    std::map< std::string, IXT::HVEC< DEVICE > >   devices;

} _internal;

int push_device( device_name_t name, IXT::HVEC< DEVICE > device ) {
    std::unique_lock lock{ _internal.devices_mtx };

    _internal.devices[ name ].vector( std::move( device ) );
    return 0;
}

std::optional< IXT::HVEC< DEVICE > > pull_device( device_name_t name ) {
    std::unique_lock lock{ _internal.devices_mtx };
    auto itr = _internal.devices.find( name );
    lock.unlock();

    if( itr == _internal.devices.end() ) return {};
    return itr->second;
}

IXT::HVEC< DEVICE > extract_device( device_name_t name ) {
    IXT::HVEC< DEVICE > ret = NULL;

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


} };