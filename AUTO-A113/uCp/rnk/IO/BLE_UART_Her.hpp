#pragma once
/**
 * @file: uCp/IO/BLE_UART_Her.hpp
 * @brief: 
 * @details:
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include "BLE_UART_Core.hpp"

namespace rnk { namespace IO { namespace BLE_UART {

class Her : RNK_PROTECTED NimBLEServerCallbacks, RNK_PROTECTED NimBLECharacteristicCallbacks {
public:
    const struct PIN_MAP {
        pin_t Q_light;
    } _pin_map;

public:
    Her( const PIN_MAP& pin_map_ = { Q_light: 0x0 } )
    : _pin_map{ pin_map_ }
    {}

RNK_PROTECTED:
    NimBLEServer*    _server     = NULL;
    NimBLEService*   _service    = NULL;
    struct _CHAR {
        NimBLECharacteristic*   h2m   = NULL;
        NimBLECharacteristic*   m2h   = NULL;
    }                 _char      = {};
    Atomic< bool >    _has_me    = { false };

    TaskHandle_t      _h_light   = NULL;

RNK_PROTECTED:
/* NimBLEServerCallbacks: */
    virtual void onConnect( NimBLEServer* server_, NimBLEConnInfo& info_ ) override { 
        _has_me.store( true ); 
    }

    virtual void onDisconnect( NimBLEServer* server_, NimBLEConnInfo& info_, int reason_ ) override { 
        _has_me.store( false ); 
    }

/* NimBLECharacteristicCallbacks: */
    virtual void onWrite( NimBLECharacteristic* char_, NimBLEConnInfo& info ) override {
        this->on_literally_me( { ptr: const_cast< uint8_t* >( char_->getValue().data() ), sz: char_->getValue().size() } );
    }

RNK_PROTECTED:
    static void _light_handler( void* arg_ ) { 
        Her* self = ( Her* )arg_;
        pinMode( self->_pin_map.Q_light, OUTPUT );
    for(;;) {
        digitalWrite( self->_pin_map.Q_light, 0x1 );
        vTaskDelay( 100 );
        digitalWrite( self->_pin_map.Q_light, 0x0 );
        vTaskDelay( self->is_uplinked() ? 3000 : 1000 );
    } }

public:
    status_t begin( const char* name_ ) {
        NimBLEDevice::init( name_ );
        
        RNK_ASSERT_OR( _server = NimBLEDevice::createServer() ) {
            return -0x1;
        }
        _server->setCallbacks( this );

        RNK_ASSERT_OR( _service = _server->createService( UUID.SERVICE ) ) {
            return -0x1;
        }

        RNK_ASSERT_OR( _char.h2m = _service->createCharacteristic( UUID.CHAR_H2M, NIMBLE_PROPERTY::NOTIFY ) ) {
            return -0x1;
        }
        RNK_ASSERT_OR( _char.m2h = _service->createCharacteristic( UUID.CHAR_M2H, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR ) ) {
            return -0x1;
        }
        _char.m2h->setCallbacks( this );

        RNK_ASSERT_OR( _service->start() ) {
            return -0x1;
        }

        NimBLEAdvertisementData advertisement_data; 
        advertisement_data.setName( name_ ); 
        advertisement_data.addServiceUUID( _service->getUUID() );

        _server->advertiseOnDisconnect( true );

        NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
        advertising->setScanResponseData( advertisement_data );
        advertising->addServiceUUID( _service->getUUID() );
        advertising->start();

        if( 0x0 != _pin_map.Q_light ) xTaskCreate( &Her::_light_handler, "rnk::IO::BLE_UART::Her::_light_handler", 1024, ( void* )this, TaskPriority_Mach, &_h_light ); 

        return 0x0;
    }


public:
    virtual void on_literally_me( MDsc mdsc_ ) = 0;

    void tell_literally_me( MDsc mdsc_ ) {
        _char.h2m->setValue( mdsc_.ptr, mdsc_.sz );
        _char.h2m->notify();
    }

    void tell_literally_me( const char* str_ ) {
        _char.h2m->setValue( str_ );
        _char.h2m->notify();
    }

public:
    bool is_uplinked( void ) const { return _has_me.load(); }

};


}; }; };