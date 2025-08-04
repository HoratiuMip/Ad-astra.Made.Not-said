#include "../../wjpv2_devices.hpp"


struct _WJP_INTER_MECH_InoSerial : WJP_BRIDGE_InterMech {
    _WJP_INTER_MECH_InoSerial( HardwareSerial* serial )
    : _serial{ serial } {}

    HardwareSerial*   _serial   = nullptr;

    _WJP_forceinline int send( WJP_MDsc_v mdsc, int flags, void* arg ) override {
        return _serial->write( ( char* )mdsc.addr, mdsc.sz ) == mdsc.sz ? mdsc.sz : -1;
    }

    _WJP_forceinline int recv( WJP_MDsc_v mdsc, int flags, void* arg ) override { int cnt = 0;
        for( int b = 0; b < mdsc.sz; ++b ) {
            int r = _serial->read();
            if( r == -1 ) { --b; continue; }
            ( ( char* )mdsc.addr )[ b ] = ( char )r; ++cnt;
        }
      
        return cnt;
    }
};


template< int RESOLVER_QUEUE_SIZE >
struct WJP_DRIVER_EUCLID_InoSerial_RTG : protected _WJP_INTER_MECH_InoSerial, protected WJP_DEVICE_Euclid_RTG {
    WJP_DRIVER_EUCLID_InoSerial_RTG( HardwareSerial* serial )
    : _WJP_INTER_MECH_InoSerial{ serial } {
        this->WJP_DEVICE_Euclid_RTG::bind_inter_mech( ( WJP_BRIDGE_InterMech* )this );
        this->WJP_DEVICE_Euclid_RTG::bind_resolvers_mdsc( WJP_MDsc< WJP_WBCK_Resolver_RTG >{ addr: _resolver_queue_mem, sz: RESOLVER_QUEUE_SIZE } );
    }

    WJP_WBCK_Resolver_RTG   _resolver_queue_mem[ RESOLVER_QUEUE_SIZE ];

    using WJP_DEVICE_Euclid_RTG::XO_heart;

};