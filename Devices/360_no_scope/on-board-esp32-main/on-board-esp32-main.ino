#include <Wire.h>


typedef   uint8_t         GPIO_pin_t;
typedef   const uint8_t   GPIO_cpin_t;
struct _GPIO {
  GPIO_cpin_t   hiFuse_Gnd     = 17;  /* N555 - 1 - Gnd connected to this pin. */
  GPIO_cpin_t   odFuse_Trig    = 5; /* N555 - 2 */
  GPIO_cpin_t   idFuse_Out     = 18; /* N555 - 3 */
  GPIO_cpin_t   odFuse_Reset   = 19; /* N555 - 4 */
  GPIO_cpin_t   hiFuse_Cont    = 32; /* N555 - 5 */
  GPIO_cpin_t   iaMsr          = 33; /* N555 - 6 */
  GPIO_cpin_t   hiFuse_Disch   = 25; /* N555 - 7 */
  GPIO_cpin_t   hiFuse_Vcc     = 26; /* N555 - 8 - Vcc connected to this pin. */
} GPIO;


struct {
  void reset() {
   
  }
} _360NS;


void setup() { 
  pinMode( GPIO.odFuse_Reset, INPUT_PULLUP );
  pinMode( GPIO.odFuse_Trig, PULLUP );

  //pinMode( GPIO.hiFuse_Gnd, INPUT ); 
  pinMode( GPIO.hiFuse_Cont, INPUT ); 
  pinMode( GPIO.hiFuse_Disch, INPUT ); 
  pinMode( GPIO.hiFuse_Vcc, INPUT ); 

  pinMode( GPIO.idFuse_Out, INPUT );
  pinMode( GPIO.iaMsr, INPUT );

  Serial.begin( 115200 ); 
}


void loop() {
  Serial.println( "Hallow" );
  if( Serial.available() ) {
    Serial.read();
    _360NS.reset();
  }
}