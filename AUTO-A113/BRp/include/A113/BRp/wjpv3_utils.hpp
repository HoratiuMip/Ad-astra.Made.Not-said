#pragma once
/**
 * @file: BRp/wjpv3_utils.hpp
 * @brief
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>
#include <A113/BRp/IO_port.hpp>

#include <wjpv3_devices.hpp>

namespace A113 { namespace BRp {


class WJPv3_LMHIPayload : public WJP_InterMech, public WJP_LMHIReceiver, public WJPDevice_Euclid {
public:
    WJPv3_LMHIPayload( IO_port* io_port_, BUFFER tx_buf_, BUFFER rx_buf_ )
    : _io_port{ io_port_ }, _tx_buf{ tx_buf_ }
    {
        this->bind_inter_mech( this );
        this->bind_lmhi_receiver( this );
        this->bind_recv_buffer( { rx_buf_.ptr, rx_buf_.n } );

        *( WJP_Head* )_tx_buf.ptr = WJP_Head{};
    }

A113_PROTECTED:
    IO_port*   _io_port   = nullptr;
    BUFFER     _tx_buf    = {};

public:
    A113_inline void bind_TX_buffer( BUFFER buf_ ) { _tx_buf = buf_; }

A113_PRIVATE:
    virtual int mech_send( WJP_MDsc mdsc_ ) final override {
        return _io_port->basic_write_loop( { mdsc_.addr, mdsc_.sz } );
    }

    virtual int mech_recv( WJP_MDsc mdsc_ ) final override {
        return _io_port->basic_read_loop( { mdsc_.addr, mdsc_.sz } );
    }

public:
    A113_inline int lmhi_tx_payload( WJPInfo_TX* info_, int N_ ) {
        return this->TX_lmhi_payload_pck( info_, ( WJP_Head* )_tx_buf.ptr, N_ );
    }

public:
    BUFFER payload( void ) { 
        return { _tx_buf.ptr + sizeof( WJP_Head ), _tx_buf.n - sizeof( WJP_Head ) };
    };

};


template< typename IO_PORT_T >
class WJPv3_LMHIPayload_on : public WJPv3_LMHIPayload, public IO_PORT_T {
public:
    WJPv3_LMHIPayload_on( BUFFER tx_buf, BUFFER rx_buf )
    : BRp::WJPv3_LMHIPayload{ this, tx_buf, rx_buf }
    {}

};

template< typename IO_PORT_T, int TX_BUF_SZ, int RX_BUF_SZ >
class WJPv3_LMHIPayload_InternalBufs_on : public WJPv3_LMHIPayload_on< IO_PORT_T > {
public:
    WJPv3_LMHIPayload_InternalBufs_on()
    : WJPv3_LMHIPayload_on< IO_PORT_T >{ { _tx_buf, TX_BUF_SZ }, { _rx_buf, RX_BUF_SZ } }
    {}

A113_PROTECTED:
    char   _tx_buf[ TX_BUF_SZ ];
    char   _rx_buf[ RX_BUF_SZ ];

};


} };
