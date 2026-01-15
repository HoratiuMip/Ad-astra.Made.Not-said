/**
 * @file: osp/IO_serial.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/IO_serial.hpp>

namespace a113::io {


status_t Serial::open( const char* device_, const serial_config_t& config_ ) {
    if( _port != SERIAL_NULL_HANDLE ) this->close();

    _port = CreateFileA( device_, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    A113_ON_SCOPE_EXIT_L( [ this ] ( void ) -> void {
        if( _port != SERIAL_NULL_HANDLE ) {
            CloseHandle( std::exchange( _port, SERIAL_NULL_HANDLE ) );
        }
    } );

    A113_ASSERT_OR( _port != SERIAL_NULL_HANDLE ) {
        A113_LOGE_IO( "Could not open serial port \"{}\", error code [{}].", device_, GetLastError() );
        return -0x1;
    }

    A113_ASSERT_OR( FlushFileBuffers( _port ) ) {
        A113_LOGW_IO( "Could not flush serial port buffers for opened serial port \"{}\", error code [{}].", device_, GetLastError() );
    }

    COMMTIMEOUTS timeouts{
        ReadIntervalTimeout         : config_.rx_ib_timeout,
        ReadTotalTimeoutMultiplier  : 0,
        ReadTotalTimeoutConstant    : config_.rx_fb_timeout,
        WriteTotalTimeoutMultiplier : 0,
        WriteTotalTimeoutConstant   : config_.tx_timeout
    };

    A113_ASSERT_OR( SetCommTimeouts( _port, &timeouts ) ) {
        A113_LOGW_IO( "Could not set the configured timeouts for serial port \"{}\", error code [{}].", device_, GetLastError() );
    }

    DCB state{ 0 }; state.DCBlength = sizeof( DCB );

    A113_ASSERT_OR( GetCommState( _port, &state ) ) {
        A113_LOGW_IO( "Could not read the default state of the serial port \"{}\". Some configurations might be defaulted or contain garbage values. Error code [{}].", device_, GetLastError() );
    }

    state.BaudRate     = ( uint32_t )config_.baud_rate;
    state.ByteSize     = config_.byte_size;
    state.Parity       = config_.parity;
    state.StopBits     = config_.stopbit;
    state.fOutxCtsFlow = false;  
    state.fRtsControl  = RTS_CONTROL_DISABLE;
    state.fOutX        = false;  
    state.fInX         = false;   

    A113_ASSERT_OR( SetCommState( _port, &state ) ) {
        A113_LOGE_IO( "Could not configure the serial port \"{}\", error code [{}].", device_, GetLastError() );
        return -0x1;
    }

    if( config_.purge_on_open ) this->purge();

    A113_ON_SCOPE_EXIT_DROP;
    _device = device_;
    _config = config_;

    A113_LOGI_IO( "Opened and configured serial port \"{}\" successfully @{}bauds.", _device, _config.baud_rate );
    return 0x0;
}

status_t Serial::close( void ) {
    if( _port == SERIAL_NULL_HANDLE ) return 0x0;
    
    if( _config.purge_on_close ) this->purge();
    CloseHandle( std::exchange( _port, INVALID_HANDLE_VALUE ) );
    A113_LOGI_IO( "Closed serial port \"{}\".", _device );
    _device = "";
    _config = serial_config_t{};
    return 0x0;
}

status_t Serial::read( const port_RW_desc_t& desc_ ) {
    uint32_t byte_count = 0;
    ReadFile( _port, desc_.ptr_SoD, desc_.n_SoD, ( LPDWORD )&byte_count, nullptr );
    
    if( desc_.byte_count ) *desc_.byte_count = byte_count;

    if( desc_.fail_if_not_all && byte_count != desc_.n_SoD ) {
        return -0x1;
    }

    return 0x0;
}

status_t Serial::write( const port_RW_desc_t& desc_ ) {
    uint32_t byte_count = 0;
    WriteFile( _port, desc_.ptr_SoD, desc_.n_SoD, ( LPDWORD )&byte_count, nullptr );

    if( desc_.byte_count ) *desc_.byte_count = byte_count;

    if( desc_.fail_if_not_all && byte_count != desc_.n_SoD ) {
        return -0x1;
    }

    return 0x0;
}

int Serial::rx_available( void ) const {
    COMSTAT stat; memset( &stat, 0x0, sizeof( COMSTAT ) );
    A113_ASSERT_OR( ClearCommError( _port, nullptr, &stat ) ) return -0x1;
    return stat.cbInQue;
}

status_t Serial::purge( void ) const {
    A113_ASSERT_OR( PurgeComm( _port, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR ) ) {
        A113_LOGW_IO( "Could not purge serial port \"{}\", error code [{}].", _device, GetLastError() );
        return -0x1;
    }
    return 0x0;
}


}
