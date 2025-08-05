#ifndef WJPV2_DRIVER_EUCLID_INO_SERIAL_HPP
#define WJPV2_DRIVER_EUCLID_INO_SERIAL_HPP
/*===== Warp Joint Protocol v2 - Driver | Euclid | Serial on Arduino - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include "../../wjpv2_devices.hpp"


struct _WJP_INTER_MECH_InoSerial : WJP_BRIDGE_InterMech {
    _WJP_INTER_MECH_InoSerial( HardwareSerial* serial )
    : _serial{ serial } {}

    HardwareSerial*   _serial   = nullptr;

    _WJP_forceinline int send( WJP_MDsc_v mdsc, int flags, void* arg ) override {
        return _serial->write( ( char* )mdsc.addr, mdsc.sz ) == mdsc.sz ? mdsc.sz : -1;
    }

    _WJP_forceinline int recv( WJP_MDsc_v mdsc, int flags, void* arg ) override { int cnt = 0;
        return _serial->readBytes( ( char* )mdsc.addr, mdsc.sz ) == mdsc.sz ? mdsc.sz : -1;
    }

    _WJP_forceinline int drain( int flags, void* arg ) override {
        while( _serial->read() != -1 );
        return 0;
    }
};


template< int RESOLVER_QUEUE_SIZE, int RECV_BUFFER_SIZE >
struct WJP_DRIVER_EUCLID_InoSerial_RTG : protected _WJP_INTER_MECH_InoSerial, protected WJP_DEVICE_Euclid_RTG {
    WJP_DRIVER_EUCLID_InoSerial_RTG( HardwareSerial* serial, WJP_BRIDGE_LMHIReceiver* lmhi_receiver )
    : _WJP_INTER_MECH_InoSerial{ serial } {
        this->WJP_DEVICE_Euclid_RTG::bind_inter_mech( ( WJP_BRIDGE_InterMech* )this );
        this->WJP_DEVICE_Euclid_RTG::bind_resolvers_mdsc( WJP_MDsc< WJP_WBCK_Resolver_RTG >{ addr: _resolver_queue_mem, sz: RESOLVER_QUEUE_SIZE } );
        this->WJP_DEVICE_Euclid_RTG::bind_recv_buffer( WJP_MDsc_v{ addr: ( void* )_recv_buffer, sz: RECV_BUFFER_SIZE } );
        this->WJP_DEVICE_Euclid_RTG::bind_lmhi_receiver( lmhi_receiver );
    }

    WJP_WBCK_Resolver_RTG   _resolver_queue_mem[ RESOLVER_QUEUE_SIZE ];
    char                    _recv_buffer[ RECV_BUFFER_SIZE ];

    using WJP_DEVICE_Euclid_RTG::XO_heart;

    int init( int baud ) {
        _WJP_INTER_MECH_InoSerial::_serial->begin( baud );
        this->_WJP_INTER_MECH_InoSerial::drain( 0, nullptr );
        return 0;
    }

    int terminate( void ) {
        _WJP_INTER_MECH_InoSerial::_serial->end();
        return 0;
    }
};


#endif