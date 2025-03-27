#include <BluetoothSerial.h>
#include <Wire.h>

#include "../../IXN/common_utils.hpp"
using namespace ixN;


typedef   int8_t   GPIO_pin_t;
typedef   int8_t   LED_RGB;
static struct _GPIO { 
  const int   ADC_nmax = 4095;
  const float ADC_fmax = 4095.0f;

  struct { const GPIO_pin_t sw, x, y; } rachel{ sw: 27, x: 35, y: 32 }, samantha{ sw: 33, x: 26, y: 25 };
  /*       JOYSTICKS                    |lower left                     |upper right */

  const GPIO_pin_t giselle = 5, karina = 18, ningning = 19, winter = 23;
  /*       SWS     |blue        |red         |yellow        |green */

  const GPIO_pin_t naksu = 4;
  /*               |light sensor */

  struct { const GPIO_pin_t r, g, b; } bitna{ r: 13, g: 12, b: 14 };

  int init() {
    _printf( LogLevel_Info, "Configuring pin modes... " );

    pinMode( rachel.sw, INPUT_PULLUP ); pinMode( rachel.x, INPUT ); pinMode( rachel.y, INPUT );
    pinMode( samantha.sw, INPUT_PULLUP ); pinMode( samantha.x, INPUT ); pinMode( samantha.y, INPUT );

    pinMode( giselle,  INPUT_PULLUP );
    pinMode( karina,   INPUT_PULLUP );
    pinMode( ningning, INPUT_PULLUP );
    pinMode( winter,   INPUT_PULLUP );

    pinMode( bitna.r, OUTPUT ); pinMode( bitna.g, OUTPUT ); pinMode( bitna.b, OUTPUT );

    _printf( "ok.\n" );

    return 0;
  }

} GPIO;


void setup( void ) {
  Serial.begin( 115200 );

  GPIO.init();
}

void loop( void ) {
  digitalWrite( 13, LOW );
    delay( 1000 );
    digitalWrite( 13, HIGH );
    delay( 1000 );
}

