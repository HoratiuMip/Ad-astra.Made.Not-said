#ifndef WJPV3_DEVICES_HPP
#define WJPV3_DEVICES_HPP
/*===== Warp Joint Protocol v3 - Devices - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Yes.
|
======*/
#include "wjpv3_core.hpp"


struct WJPInfo_TX {
    WJPErr_   err   = WJPErr_None;
};

struct WJPInfo_RX {
    WJPErr_   err   = WJPErr_None;
};


struct WJPDevice_Euclid {
    inline static const int _PHASE_BUFFER_SIZE = 18;
    inline static const unsigned char _PHASE_BUFFER[ _PHASE_BUFFER_SIZE ] = {
        'W', 'J', 'P', '3',
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        'W', 'J', 'P', '3'
    };

    WJP_InterMech*      _inter_mech      = nullptr;
    WJP_MDsc            _recv_mdsc       = {};

    WJP_LMHIReceiver*   _lmhi_receiver   = nullptr;

    void bind_inter_mech( WJP_InterMech* inter_mech_ ) { _inter_mech = inter_mech_; }
    void bind_recv_buffer( WJP_MDsc mdsc_ = {} ) { _recv_mdsc = mdsc_; }
    void bind_lmhi_receiver( WJP_LMHIReceiver* lmhi_receiver_ ) { _lmhi_receiver = lmhi_receiver_; }

#define _WJP_INTER_MECH_INFO_ERR( r, e ) { info_->err = ( (r) == 0 ? WJPErr_Reset : (e) ); } 

    int TX_lmhi_head_exp( WJPInfo_TX* info_, int16_t ACT_, int32_t ARG_, int32_t N_ ) {
        WJP_Head head = {}; 

        head._dw1.ACT = ACT_; head._dw2.ARG = ARG_; head._dw3.N = N_;
        head.reset_payload();
        head.set_lmhi();

        int result = _inter_mech->mech_send( { ( char* )&head, sizeof( WJP_Head ) } );
        _WJP_ASSERT_OR( sizeof( WJP_Head ) == result ) {
            _WJP_INTER_MECH_INFO_ERR( result, WJPErr_TX );
            return result;
        }

        return result;
    }

    int TX_lmhi_payload_pck( WJPInfo_TX* info_, WJP_Head* head_, int32_t N_ = 0 ) {
        if( N_ > 0 ) head_->_dw3.N = N_;

        head_->set_payload();
        head_->set_lmhi();

        int acc_size = sizeof( WJP_Head ) + head_->_dw3.N;

        int result = _inter_mech->mech_send( { ( char* )head_, acc_size } );
        _WJP_ASSERT_OR( acc_size == result ) {
            _WJP_INTER_MECH_INFO_ERR( result, WJPErr_TX );
            return result;
        }

        return result;
    }

    int RX_lmhi( WJPInfo_RX* info_ ) {
        WJP_LMHIReceiver::Layout layout;

        int result = _inter_mech->mech_recv( { ( char* )&layout.head_in, sizeof( WJP_Head ) } );
        _WJP_ASSERT_OR( sizeof( WJP_Head ) == result ) {
            _WJP_INTER_MECH_INFO_ERR( result, WJPErr_RX );
            return result;
        }

        _WJP_ASSERT_OR( layout.head_in.is_aligned() && layout.head_in.is_lmhi() ) { info_->err = WJPErr_Ctl; return -0x1; }

        if( layout.head_in.is_payload() ) {
            int32_t N = layout.head_in._dw3.N;
            _WJP_ASSERT_OR( N > 0x0 && N <= _recv_mdsc.sz ) { info_->err = WJPErr_N; return -0x1; }
            result = _inter_mech->mech_recv( { ( char* )_recv_mdsc.addr, N } );
            _WJP_ASSERT_OR( N == result ) { _WJP_INTER_MECH_INFO_ERR( result, WJPErr_RX ); return -0x1; }

            layout.payload_in.addr = _recv_mdsc.addr;
            layout.payload_in.sz   = N;
        }

        return _lmhi_receiver->lmhi_when_recv( &layout );
    }

};


#endif