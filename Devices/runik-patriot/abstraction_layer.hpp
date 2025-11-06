#pragma once
/**
 * @file 
 * @brief 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <rnk/IO/BLE_UART_Literally_me.hpp>
#include <wjpv3.hpp>

namespace rp {


inline const char*   TAG            = "Runik-Patriot";
inline const char*   TAG_EYE        = "Runik-Patriot-Eye";
inline const char*   TAG_EYE_SSID   = "Runik-Patriot";
inline const char*   TAG_EYE_PWD    = "runikpisik"; /* oh no! do not steal the password! */
inline const char*   TAG_EYE_DN     = "runik";


enum WJPAct_ {
    WJPAct_VirtualCommander
};

#define RP_TRACK_MODE_DECOUPLED 0x0
#define RP_TRACK_MODE_DIFFERENTIAL 0x1


struct virtual_commander_t {
    float   track_left; 
    float   track_right;
    float   track_pwr;
    uint8_t track_mode;

    float headlight_left;
    float headlight_right;
};

class BLE_UART_Virtual_commander : RNK_PROTECTED rnk::IO::BLE_UART::Literally_me, RNK_PROTECTED WJPDevice_Euclid, RNK_PROTECTED WJP_InterMech {
public:
    BLE_UART_Virtual_commander() {
        this->WJPDevice_Euclid::bind_inter_mech( this );
    }

RNK_PROTECTED:
    struct _payload_t {
        inline static constexpr WJP_size_t   TOTAL_SIZE   = sizeof( WJP_Head ) + sizeof( virtual_commander_t ); 

        _payload_t() {
            head._dw1.ACT = rp::WJPAct_VirtualCommander;
            head._dw3.N = sizeof( virtual_commander_t );
        } 

        WJP_Head   head;
        uint8_t    data[ sizeof( virtual_commander_t ) ];
    } _payload;

RNK_PROTECTED:
/* rnk::IO::BLE_UART::Literally_me: */
    virtual void on_her( rnk::MDsc mdsc_ ) override {

    }

/* WJP_InterMech: */
    virtual WJP_result_t WJP_mech_send( WJP_MDsc mdsc_ ) override {
        this->rnk::IO::BLE_UART::Literally_me::tell_her( rnk::MDsc{ ( uint8_t* )mdsc_.addr, mdsc_.sz } );
        return 0x0;
    }

    virtual WJP_result_t WJP_mech_recv( WJP_MDsc mdsc_ ) override {
        return -0x1;
    }

public:
    rnk::status_t push( const virtual_commander_t& vcmd_ ) {
        memcpy( _payload.data, &vcmd_, sizeof( virtual_commander_t ) );
        WJPInfo_TX info;
        return this->WJPDevice_Euclid::TX_lmhi_payload_pck( &info, &_payload.head );
    }

public:
    using rnk::IO::BLE_UART::Literally_me::begin;
    using rnk::IO::BLE_UART::Literally_me::uplink;
    using rnk::IO::BLE_UART::Literally_me::downlink;

};


};