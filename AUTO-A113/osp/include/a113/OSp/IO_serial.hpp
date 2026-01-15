#pragma once
/**
 * @file: osp/IO_serial.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>

#include <a113/brp/IO_port.hpp>
#include <a113/brp/IO_string_utils.hpp>

#ifdef A113_TARGET_OS_WINDOWS
    typedef   ::HANDLE   serial_handle_t;

    inline const serial_handle_t   SERIAL_NULL_HANDLE       = INVALID_HANDLE_VALUE;

    inline constexpr uint8_t       SERIAL_PARITY_NONE       = NOPARITY; 
    inline constexpr uint8_t       SERIAL_PARITY_ODD        = ODDPARITY; 
    inline constexpr uint8_t       SERIAL_PARITY_EVEN       = EVENPARITY; 
    inline constexpr uint8_t       SERIAL_PARITY_MARK       = MARKPARITY; 
    inline constexpr uint8_t       SERIAL_PARITY_SPACE      = SPACEPARITY; 

    inline constexpr uint8_t       SERIAL_STOPBIT_ONE       = ONESTOPBIT;
    inline constexpr uint8_t       SERIAL_STOPBIT_ONE_HALF  = ONE5STOPBITS;
    inline constexpr uint8_t       SERIAL_STOPBIT_TWO       = TWOSTOPBITS;
#endif 

namespace a113::io { 

struct serial_config_t {
    uint32_t   baud_rate       = 0;
    uint32_t   rx_fb_timeout   = 1000;
    uint32_t   rx_ib_timeout   = 10;
    uint32_t   tx_timeout      = 1000;
    uint8_t    byte_size       = 8;
    uint8_t    parity          = SERIAL_PARITY_NONE;
    uint8_t    stopbit         = SERIAL_STOPBIT_ONE;
    bool       purge_on_open   = true;
    bool       purge_on_close  = false;
};

class Serial : public Port {
public:
    Serial() = default;

    Serial( const char* device_, const serial_config_t& config_ ) {
        this->open( device_, config_ );
    }

    ~Serial() {
        this->close();
    }

_A113_PROTECTED:
    serial_handle_t   _port       = SERIAL_NULL_HANDLE;
    std::string       _device     = "";
    serial_config_t   _config     = {};

public:
    A113_inline serial_handle_t native_handle( void ) const { return _port; }

    A113_inline std::string_view device( void ) const { return _device; }

public:
    A113_inline bool is_connected( void ) const { return _port != SERIAL_NULL_HANDLE; }

public:
    status_t open( const char* device_, const serial_config_t& config_ );

    status_t close( void );

public:
    int read( const port_RW_desc_t& desc_ ) override;

    int write( const port_RW_desc_t& desc_ ) override;

public:
    int rx_available( void ) const;

    status_t purge( void ) const;

};

}



