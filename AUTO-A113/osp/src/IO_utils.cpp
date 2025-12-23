/**
 * @file: OSp/IO_utils.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/IO_utils.hpp>

namespace a113::io {

    
static status_t _populate_ports( COM_Ports::container_t& ports_ ) {
    HDEVINFO dev_set = SetupDiGetClassDevs( &GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT );
    A113_ASSERT_OR( dev_set != INVALID_HANDLE_VALUE ) return -0x1;

    SP_DEVINFO_DATA dev_data{
        .cbSize = sizeof( SP_DEVINFO_DATA )
    };
    int  count = 0;
    char buffer[ 256 ];

    ports_.clear();

    for( int n = 0; SetupDiEnumDeviceInfo( dev_set, n, &dev_data ); ++n ) {
        if( !SetupDiGetDeviceRegistryPropertyA( dev_set, &dev_data, SPDRP_FRIENDLYNAME, NULL, ( PBYTE )buffer, sizeof( buffer ), NULL ) ) continue;
        ++count;
        auto& port = ports_.emplace_back( COM_port_t{ 
            .id       = "COM", 
            .friendly = buffer
        } );

        char* ptr = strstr( buffer, "COM" ); ptr += 0x3;
        while( *ptr >= '0' && *ptr <= '9' ) port.id += *( ptr++ );
    }

    SetupDiDestroyDeviceInfoList( dev_set );
    return count;
}

static DWORD CALLBACK _listen_callback (
    [[maybe_unused]]HCMNOTIFICATION,
    PVOID                            self_,
    CM_NOTIFY_ACTION                 action_,
    PCM_NOTIFY_EVENT_DATA            event_,
    [[maybe_unused]]DWORD
) {
    if( action_ != CM_NOTIFY_ACTION_DEVICEINTERFACEARRIVAL && action_ != CM_NOTIFY_ACTION_DEVICEINTERFACEREMOVAL ) return 0x0;

    if( event_->FilterType != CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE ) return 0x0;

    if( not IsEqualGUID( event_->u.DeviceInterface.ClassGuid, GUID_DEVINTERFACE_COMPORT ) ) return 0x0;
            
    auto* self = ( COM_Ports* )self_;

    self->refresh();

    return 0x0;
}

A113_IMPL_FNC COM_Ports& COM_Ports::refresh( void ) {
    status_t status = -0x1;

    switch( this->disp_mode() ) {
        case DispenserMode_Lock: {
            std::lock_guard lock{ *this };
            status = _populate_ports( **this );
        break; }

        case DispenserMode_Swap: {
            auto ports = HVec< container_t >::make();
            status = _populate_ports( *ports );
            this->HVec< container_t >::operator=( std::move( ports ) );
        break; }

        default: {
            _Log::error( "Bad dispenser mode." );
            return *this;
        }
    }

    if( status > 0x0 ) _Log::info( "Refreshed, found [{}] port(s).", status );
    else if( status == 0x0 ) _Log::info( "Refreshed, no ports found." );
    else _Log::error( "Bad refresh // [{}].", status );

    return *this;
}

A113_IMPL_FNC status_t COM_Ports::register_listen( void ) {
    CM_NOTIFY_FILTER filter {
        .cbSize                         = sizeof( CM_NOTIFY_FILTER ),
        .FilterType                     = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE,
        .u{.DeviceInterface={.ClassGuid = GUID_DEVINTERFACE_COMPORT}}
    };

    return CM_Register_Notification( &filter, ( PVOID )this, &_listen_callback, &_notif ) == CR_SUCCESS ? 0x0 : -0x1;
}

A113_IMPL_FNC status_t COM_Ports::unregister_listen( void ) {
    return CM_Unregister_Notification( _notif ) == CR_SUCCESS ? 0x0 : -0x1;
}


}