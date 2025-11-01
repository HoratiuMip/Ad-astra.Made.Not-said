#pragma once
/**
 * @file: uCp/IO/BLE_UART_Her.hpp
 * @brief: 
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "BLE_UART_Core.hpp"

namespace rnk { namespace IO { namespace BLE_UART {

class Literally_me {
RNK_PROTECTED:
    NimBLEClient*          _client    = NULL;
    NimBLERemoteService*   _service   = NULL;
    struct _CHAR {
        NimBLERemoteCharacteristic*   h2m   = NULL;
        NimBLERemoteCharacteristic*   m2h   = NULL;
    }                      _char      = {};

public:
    status_t begin( void ) {
        NimBLEDevice::init( "" );
        
        RNK_ASSERT_OR( _client = NimBLEDevice::createClient() ) {
            return -0x1;
        }

        return 0x0;
    }

public:
    status_t uplink( const char* her_, uint16_t scan_ms_ ) {
        NimBLEScan* scanner = NimBLEDevice::getScan();
        RNK_ASSERT_OR( scanner ) {
            return -0x1;
        }

        scanner->setActiveScan( true );
        scanner->start( 0, false );
        vTaskDelay( scan_ms_ );
        scanner->stop();

        NimBLEScanResults             results = scanner->getResults();
        const NimBLEAdvertisedDevice* device  = NULL;

        for( int idx = 0; idx < results.getCount(); ++idx ) {
            device = results.getDevice( idx );
            if( device->getName() == her_ ) break;
        }

        RNK_ASSERT_OR( device ) {
            return -0x1;
        }

        RNK_ASSERT_OR( _client->connect( device ) ) {
            return -0x1;
        } 

        RNK_ASSERT_OR( _service = _client->getService( UUID.SERVICE ) ) {
            return -0x1;
        }
        RNK_ASSERT_OR( _char.h2m = _service->getCharacteristic( UUID.CHAR_H2M ) ) {
            return -0x1;
        }
        RNK_ASSERT_OR( _char.m2h = _service->getCharacteristic( UUID.CHAR_M2H ) ) {
            return -0x1;
        }

        _char.h2m->subscribe( true, [ this ] ( NimBLERemoteCharacteristic* char_, uint8_t* data_, size_t size_, bool is_notify_ ) -> void {
            this->on_her( { ptr: data_, sz: size_ } );
        } );

        return 0x0;
    }

    status_t downlink( void ) {
        return _client->disconnect() ? 0x0 : -0x1;
    }
    
public:
    virtual void on_her( MDsc mdsc_ ) = 0;

    void tell_her( MDsc mdsc_ ) {
        _char.m2h->writeValue( mdsc_.ptr, mdsc_.sz );
    }

    void tell_her( const char* str_ ) {
        _char.m2h->writeValue( str_ );
    }

public:
    bool is_uplinked( void ) const { return _client ? _client->isConnected() : false; }

};


}; }; };