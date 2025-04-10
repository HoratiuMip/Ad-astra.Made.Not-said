#pragma once
/*====== IXT-NLN uC Engine - WJP on BluetoothSerial driver - Vatca "Mipsan" Tudor-Horatiu
|
>
|
======*/
#include "../common_utils.hpp"
#define WJP_ENVIRONMENT_UC
#include "../../../WJP/wjp.hpp"
#include "BluetoothSerial.h"


namespace ixN { namespace uC {


class WJP_on_BluetoothSerial : public WJP_DEVICE, public BluetoothSerial {
public:
    inline static const char* const   WHO_AM_I_STR   = "ixN::Ino::WJP_on_BluetoothSerial";

protected:
    std::atomic_int16_t   _wjp_seq                 = { 0 };
    char                  _wjp_recv_buf[ 256 ];
    int                   _blue_recv_err_delay     = 1;
    int                   _blue_recv_err_timeout   = 3000;

protected:
    int _blue_itr_recv( WJP_BUFFER dst ) {
        int count       = 0;
        int timeout_acc = 0;
    
        do {
        l_read:
            int b = this->BluetoothSerial::read();

        if( b == -1 ) { 
            int delay_ms = min( _blue_recv_err_delay, _blue_recv_err_timeout - timeout_acc );

            delay( delay_ms ); 
            timeout_acc += delay_ms;

            if( timeout_acc >= _blue_recv_err_timeout ) {
                _printf( LogLevel_Error, WHO_AM_I_STR, "Bluetooth recv exceeded set timeout (%dms).\n", _blue_recv_err_timeout );
                return -1;
            }

            goto l_read;
        } else if( timeout_acc != 0 ) {
            timeout_acc = 0;
        }

        ( ( uint8_t* )dst.addr )[ count ] = ( uint8_t )b;
        } while( ++count < dst.sz );
    
        return count;
    }

    int _blue_itr_send( WJP_CBUFFER src ) {
        int count = 0;
    
        do {
            int ret = this->BluetoothSerial::write( ( const uint8_t* )src.addr + count, src.sz - count );
            if( ret <= 0 ) return ret;
            count += ret;
        } while( count < src.sz );

        return count;
    }

public:
    int init( int flags ) {
        _printf( LogLevel_Info, WHO_AM_I_STR, "WJP init... " );

        this->WJP_DEVICE::bind_srwrap( WJP_SRWRAP{
            send: [ this ] WJP_SEND_LAMBDA { return this->_blue_itr_send( src ); },
            recv: [ this ] WJP_RECV_LAMBDA { return this->_blue_itr_recv( dst ); }
        } );

        this->WJP_DEVICE::bind_seq_acq( [ this ] () -> int16_t { return _wjp_seq.fetch_add( 1, std::memory_order_relaxed ); } );

        this->WJP_DEVICE::bind_recv_buf( WJP_BUFFER{ addr: _wjp_recv_buf, sz: sizeof( _wjp_recv_buf ) } );

        int status = this->WJP_DEVICE::open( flags );
        return status == 0 ? ( _printf( "ok.\n" ), 0 ) : ( _printf( "fault (%d).\n", status ), status );
    }

public:
    int begin( const char* bth_name ) {
        return this->BluetoothSerial::begin( bth_name ) ? 0 : -1;
    }

    int end() {
        this->BluetoothSerial::end();
        return 0;
    }

};


} };