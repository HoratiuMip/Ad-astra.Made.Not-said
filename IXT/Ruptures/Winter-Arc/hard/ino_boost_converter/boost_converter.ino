const struct _IO {
  const struct _PIN {
    const int   ADC_REZ   = 1023;

    uint8_t   OUT_BOOST_PWM       = 3;
    uint8_t   IN_BOOST_PWM_FREQ   = A1;

    int setup() {
      pinMode( OUT_BOOST_PWM, OUTPUT );
      pinMode( IN_BOOST_PWM_FREQ, INPUT );

      return 0;
    }

  } pin;

} IO;

struct _BOOST_PWM {
  double   freq   = 56e3;
  double   duty   = 0.1;

  double     _T      = 0.0;
  uint32_t   _ton    = 0.0;
  uint32_t   _toff   = 0.0;

  inline void calibrate() {
    _T    = 1.0 / freq;
    _ton  = duty * _T * 1e6;
    _toff = ( 1.0 - duty ) * _T * 1e6;
  }

  inline void refresh() {
    this->freq = ( double )analogRead( IO.pin.IN_BOOST_PWM_FREQ ) / IO.pin.ADC_REZ;
    this->freq *= 64e3;
    this->calibrate();
  }

  int setup() {
    this->calibrate();

    return 0;
  }
 
} boost_pwm;

void setup() {
  int result;
  result = IO.pin.setup();
  result = boost_pwm.setup();

  TCCR1A = 0;
  TCCR1B = 0b00000101;
  TIMSK1 = ( 1 << TOIE1 );
}

ISR( TIMER1_OVF_vect ) {
  boost_pwm.refresh();
}

void loop() {
    noInterrupts();

    delayMicroseconds( boost_pwm._ton );
    digitalWrite( IO.pin.OUT_BOOST_PWM, HIGH );
    
    delayMicroseconds( boost_pwm._toff );
    digitalWrite( IO.pin.OUT_BOOST_PWM, LOW );
    
    interrupts();
}




