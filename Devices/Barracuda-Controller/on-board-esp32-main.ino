#include "BluetoothSerial.h"

#define GPIO_D_RACHEL_S 34
#define GPIO_A_RACHEL_X 35
#define GPIO_A_RACHEL_Y 32 

#define GPIO_D_SAMANTHA_S 33
#define GPIO_A_SAMANTHA_X 26
#define GPIO_A_SAMANTHA_Y 25

#define GPIO_GREEN_SW  21
#define GPIO_YELLOW_SW 19
#define GPIO_RED_SW    22
#define GPIO_BLUE_SW    5

BluetoothSerial SerialBT;

struct barracuda_controller_state_descriptor_t {
  struct _switch_t {
      int8_t dwn      = 0;
      int8_t prs      = 0;
      int8_t rls      = 0;
      int8_t reserved = 0;
  };
  static_assert( sizeof( _switch_t ) == sizeof( int32_t ) );

  struct _joystick_t {
      _switch_t sw = {};
      float     x  = 0.0f;
      float     y  = 0.0f;
  };
  static_assert( sizeof( _joystick_t ) == sizeof( _switch_t ) + 2 * sizeof( float ) );

  barracuda_controller_state_descriptor_t() : _size{ sizeof( barracuda_controller_state_descriptor_t ) } {}

  const int32_t   _size;
  _switch_t     sw_b, sw_r, sw_y, sw_g;
  _joystick_t   rachel, samantha;

  void update() {
    rachel.x = analogRead( GPIO_A_RACHEL_X );
    rachel.y = analogRead( GPIO_A_RACHEL_Y );

    samantha.x = analogRead( GPIO_A_SAMANTHA_X );
    samantha.y = analogRead( GPIO_A_SAMANTHA_Y );

    rachel.x -= 1870.0f;
    rachel.y -= 1890.0f;

    if( rachel.x < 0.0f )
      rachel.x /= 1870.0f;
    else 
      rachel.x /= 4095.0f - 1870.0f;
    
    if( rachel.y < 0.0f )
      rachel.y /= 1890.0f;
    else 
      rachel.y /= 4095.0f - 1890.0f;

    samantha.x -= 1814.0f;
    samantha.y -= 1857.0f;

    if( samantha.x < 0.0f )
      samantha.x /= 1814.0f;
    else 
      samantha.x /= 4095.0f - 1814.0f;
    
    if( samantha.y < 0.0f )
      samantha.y /= 1857.0f;
    else 
      samantha.y /= 4095.0f - 1857.0f;
    samantha.y *= -1.0f;
    
    _update_switch_state( sw_b, GPIO_BLUE_SW );
    _update_switch_state( sw_g, GPIO_GREEN_SW );
    _update_switch_state( sw_r, GPIO_RED_SW );
    _update_switch_state( sw_y, GPIO_YELLOW_SW );
    _update_switch_state( rachel.sw, GPIO_D_RACHEL_S );
    _update_switch_state( samantha.sw, GPIO_D_SAMANTHA_S );
  }

  void _update_switch_state( _switch_t &sw, uint8_t pin ) {
    uint8_t state;

    state = !digitalRead( pin );

    switch( ( sw.dwn << 1 ) | state ) {
      case 0:
        sw.rls = 0;
        sw.prs = 0;
        break;
      case 1:
        sw.rls = 0;
        sw.prs = 1;
        break;
      case 2:
        sw.rls = 1;
        sw.prs = 0;
        break;
      case 3:
        sw.rls = 0;
        sw.prs = 0;
        break;
    }

    sw.dwn = state;
  }

  void init() {
    pinMode( GPIO_D_RACHEL_S, INPUT_PULLUP );
    pinMode( GPIO_A_RACHEL_Y, INPUT );
    pinMode( GPIO_A_RACHEL_X, INPUT );

    pinMode( GPIO_D_SAMANTHA_S, INPUT_PULLUP );
    pinMode( GPIO_A_SAMANTHA_Y, INPUT );
    pinMode( GPIO_A_SAMANTHA_X, INPUT );

    pinMode( GPIO_GREEN_SW,  INPUT_PULLUP );
    pinMode( GPIO_YELLOW_SW, INPUT_PULLUP );
    pinMode( GPIO_RED_SW,    INPUT_PULLUP );
    pinMode( GPIO_BLUE_SW,   INPUT_PULLUP );
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
      sw_g.dwn, sw_y.dwn, sw_r.dwn, sw_b.dwn
    );

    Serial.println( buffer );
  }

  void send_state_to_bluetooth() {
    SerialBT.write( ( uint8_t* ) this, sizeof( *this ) );
  }
} state_descriptor;

static_assert( sizeof( barracuda_controller_state_descriptor_t ) == 
    sizeof( int32_t ) // Structure size for compatibility.
    +
    4 * sizeof( barracuda_controller_state_descriptor_t::_switch_t ) // 4 switches.
    +
    2 * sizeof( barracuda_controller_state_descriptor_t::_joystick_t ) // 2 joysticks.
);

void setup() {
    Serial.begin( 115200 );

    state_descriptor.init();

    SerialBT.begin( "BARRACUDA" );
    Serial.println( "Bluetooth started. Waiting for connection..." );
}

void loop() {
    if ( SerialBT.available() ) {
        state_descriptor.update();
        state_descriptor.send_state_to_bluetooth();
        state_descriptor.print_to_serial();

        delay(20);
    }
}
