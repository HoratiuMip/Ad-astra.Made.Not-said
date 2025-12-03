#pragma once
/**
 * @file: BRp/wjpv3_utils.hpp
 * @brief
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/brp/descriptor.hpp>
#include <a113/brp/IO_port.hpp>

#include <wjpv3_devices.hpp>

namespace a113 { namespace wjpv3 {


class LMHIPayload : public WJP_InterMech, public WJP_LMHIReceiver, public WJPDevice_Euclid {
public:
    LMHIPayload( io::Port* io_port_, MDsc tx_buf_, MDsc rx_buf_ )
    : _io_port{ io_port_ }, _tx_buf{ tx_buf_ }
    {
        this->bind_inter_mech( this );
        this->bind_lmhi_receiver( this );
        this->bind_recv_buffer( { rx_buf_.ptr, rx_buf_.n } );

        *( WJP_Head* )_tx_buf.ptr = WJP_Head{};
    }

_A113_PROTECTED:
    io::Port*   _io_port   = nullptr;
    MDsc        _tx_buf    = {};

public:
    A113_inline void bind_TX_buffer( MDsc mdsc_ ) { _tx_buf = mdsc_; }

_A113_PRIVATE:
    virtual int WJP_mech_send( WJP_MDsc mdsc_ ) final override {
        return _io_port->basic_write_loop( { mdsc_.addr, mdsc_.sz } );
    }

    virtual int WJP_mech_recv( WJP_MDsc mdsc_ ) final override {
        return _io_port->basic_read_loop( { mdsc_.addr, mdsc_.sz } );
    }

public:
    A113_inline int lmhi_tx_payload( WJPInfo_TX* info_, int N_ ) {
        return this->TX_lmhi_payload_pck( info_, ( WJP_Head* )_tx_buf.ptr, N_ );
    }

public:
    MDsc payload( void ) { 
        return { _tx_buf.ptr + sizeof( WJP_Head ), _tx_buf.n - sizeof( WJP_Head ) };
    };

};


template< typename IO_PORT_T >
class LMHIPayload_on : public LMHIPayload, public IO_PORT_T {
public:
    LMHIPayload_on( MDsc tx_buf, MDsc rx_buf )
    : LMHIPayload{ this, tx_buf, rx_buf }
    {}

};

template< typename IO_PORT_T, int TX_BUF_SZ, int RX_BUF_SZ >
class LMHIPayload_InternalBufs_on : public LMHIPayload_on< IO_PORT_T > {
public:
    LMHIPayload_InternalBufs_on()
    : LMHIPayload_on< IO_PORT_T >{ { _tx_buf, TX_BUF_SZ }, { _rx_buf, RX_BUF_SZ } }
    {}

_A113_PROTECTED:
    char   _tx_buf[ TX_BUF_SZ ];
    char   _rx_buf[ RX_BUF_SZ ];

};


} };
