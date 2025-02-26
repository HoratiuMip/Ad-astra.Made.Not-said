#define BARRACUDA_CTRL_BUILD_FOR_ON_BOARD
#include "../../../IXT/Include/IXT/SpecMod/barracuda-ctrl.hpp"

#include "BluetoothSerial.h"


typedef   const int8_t   GPIO_pin_t;
struct {
  struct { GPIO_pin_t sw, x, y; } rachel{ sw: 34, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
  /* JOYSTICKS                    |lower left                     |upper right */

  GPIO_pin_t giselle = 5, karina = 22, ningning = 19, winter = 21;
  /* SWS     |blue        |red         |yellow        |green */

  void init() {
    pinMode( rachel.sw, INPUT_PULLUP ); pinMode( rachel.x, INPUT ); pinMode( rachel.y, INPUT );

    pinMode( samantha.sw, INPUT_PULLUP ); pinMode( samantha.x, INPUT ); pinMode( samantha.y, INPUT );

    pinMode( giselle,  INPUT_PULLUP );
    pinMode( karina,   INPUT_PULLUP );
    pinMode( ningning, INPUT_PULLUP );
    pinMode( winter,   INPUT_PULLUP );
  }

} GPIO;


struct Super : barracuda_ctrl::proto_head_t, barracuda_ctrl::state_desc_t {
  BluetoothSerial   blue_device;

  void init() {
    blue_device.begin( barracuda_ctrl::DEVICE_NAME );
    barracuda_ctrl::proto_head_t::_dw0.op = barracuda_ctrl::PROTO_OP_CODE_DESC;
    barracuda_ctrl::proto_head_t::_dw1 = sizeof( barracuda_ctrl::state_desc_t ); 
  }

  void scan() {
    rachel.x = analogRead( GPIO.rachel.x ); rachel.y = analogRead( GPIO.rachel.y );
    samantha.x = analogRead( GPIO.samantha.x ); samantha.y = analogRead( GPIO.samantha.y );

    const auto _resolve_joystick_axis = [] ( float& read, float idle_read ) -> void {
      if( ( read -= idle_read ) < .0 ) 
        read /= idle_read;
      else
        read /= 4095.0 - idle_read;
    };

    _resolve_joystick_axis( rachel.x, 1870.0 ); _resolve_joystick_axis( rachel.y, 1890.0 );
    _resolve_joystick_axis( samantha.x, 1814.0 ); _resolve_joystick_axis( samantha.y, 1857.0 );

    samantha.y *= -1.0;
    

    const auto _resolve_switch = [] ( barracuda_ctrl::switch_t& sw, GPIO_pin_t pin ) -> void {
      int is_dwn = !digitalRead( pin );

      switch( ( sw.dwn << 1 ) | is_dwn ) {
        case 0: [[fallthrough]];
        case 3: sw.rls = 0; sw.prs = 0; break;
        case 1: sw.rls = 0; sw.prs = 1; break;
        case 2: sw.rls = 1; sw.prs = 0; break;
      }

      sw.dwn = is_dwn;
    };

    _resolve_switch( rachel.sw,   GPIO.rachel.sw );
    _resolve_switch( samantha.sw, GPIO.samantha.sw );
    _resolve_switch( giselle,     GPIO.giselle );
    _resolve_switch( karina,      GPIO.karina );
    _resolve_switch( ningning,    GPIO.ningning );
    _resolve_switch( winter,      GPIO.winter);
  }

  void blue_tx_desc() {
    blue_device.write( ( uint8_t* ) this, sizeof( *this ) );
  }

  void print_to_serial() {
    char buffer[ 256 ];

    sprintf( 
      buffer, 
      "Rachel( S, X, Y ) = ( %d, %f, %f )\n" \
      "Samantha( S, X, Y ) = ( %d, %f, %f )\n" \
      "Switches( G, Y, R, B ) = ( %d, %d, %d, %d )\0",
      rachel.sw.dwn, rachel.x, rachel.y,
      samantha.sw.dwn, samantha.x, samantha.y,
      karina.dwn, karina.dwn, karina.dwn, karina.dwn
    );

    Serial.println( buffer );
  }
} super;


void setup() {
    GPIO.init();
    super.init();

    Serial.begin( 115200 );

    Serial.println( "Bluetooth started. Waiting for connection..." );
}

void loop() {
    if ( super.blue_device.available() ) {
        super.scan();
        super.blue_tx_desc();
        super.print_to_serial();

        delay(20);
    }
}
