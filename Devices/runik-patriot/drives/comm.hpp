#pragma once
/**
 * @file: comms.hpp
 * @brief:
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "cli.hpp"
#include "hyper.hpp"

#include <rnk/IO/BLE_UART_Her.hpp>
#include <wjpv3.hpp>

namespace rp {


class Comm_drive : RNK_PROTECTED rnk::IO::BLE_UART::Her, RNK_PROTECTED WJPDevice_Euclid, RNK_PROTECTED WJP_InterMech, RNK_PROTECTED WJP_LMHIReceiver {
public:
    const struct PIN_MAP {
    } _pin_map;

public:
    Comm_drive(
        const PIN_MAP&                         pin_map_,
        const rnk::IO::BLE_UART::Her::PIN_MAP& her_pin_map_,
        Cli_drive*                             cli_drive_,
        Hyper_drive*                           hyper_drive_
    )
    : rnk::IO::BLE_UART::Her{ her_pin_map_ }, _pin_map{ pin_map_ }, _cli_drive{ cli_drive_ }, _hyper_drive{ hyper_drive_ }
    {
        this->WJPDevice_Euclid::bind_inter_mech( this );
        this->WJPDevice_Euclid::bind_lmhi_receiver( this );
    }

RNK_PROTECTED:
    Cli_drive*     _cli_drive     = NULL;
    Hyper_drive*   _hyper_drive   = NULL;

public:
    rnk::status_t begin( const char* blue_name_ ) {
        this->rnk::IO::BLE_UART::Her::begin( blue_name_ );
        return 0x0;
    }

RNK_PROTECTED:
/* rnk::IO::BLE_UART::Her: */
    virtual void on_literally_me( rnk::MDsc mdsc_ ) override {
        WJPInfo_RX info;
        WJP_result_t result = this->WJPDevice_Euclid::WJP_RX_lmhi( &info, WJP_MDsc{ addr: ( char* )mdsc_.ptr, sz: mdsc_.sz } );

        if( 0x0 == result ) return;
        
        std::string str{ ( char* )mdsc_.ptr, mdsc_.sz };
        rnk::Log.info( "Received packet over BLE of size %d failed the WJPv3 routine. Continuing to interpret it as \"%s\".", mdsc_.sz, str.c_str() );
        auto [ status, msg ] = _cli_drive->exec( str );
        this->rnk::IO::BLE_UART::Her::tell_literally_me( msg.c_str() );
    }

/* WJP_InterMech: */
    virtual WJP_result_t WJP_mech_send( WJP_MDsc mdsc_ ) override {
        this->rnk::IO::BLE_UART::Her::tell_literally_me( rnk::MDsc{ ( uint8_t* )mdsc_.addr, mdsc_.sz } );
        return 0x0;
    }

    virtual WJP_result_t WJP_mech_recv( WJP_MDsc mdsc_ ) override {
        return -0x1;
    }

/* WJP_LMHIReceiver: */
    virtual WJP_result_t WJP_lmhi_when_recv( WJP_LMHIReceiver::Layout* lo_ ) override {
        switch( lo_->head_in._dw1.ACT ) {
            case WJPAct_VirtualCommander: {
                _hyper_drive->push_virtual_commander( *( virtual_commander_t* )lo_->payload_in.addr, VCMDSource_Blue );
            break; }
        }

        return 0x0;
    }

public:
    using rnk::IO::BLE_UART::Her::is_uplinked;

};


};
