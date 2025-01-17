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
  inline static const double TOP_FREQ = 54e3;

  double   freq   = TOP_FREQ;
  double   duty   = 0.92;

  uint32_t   _T      = 0.0;
  uint32_t   _ton    = 0.0;
  uint32_t   _toff   = 0.0;

  inline void calibrate() {
    _T    = ( 1.0 / freq ) * 1e6;
    _ton  = ( duty * _T );
    _toff = ( ( 1.0 - duty ) * _T );
  }

  inline void refresh() {
    analogRead( IO.pin.IN_BOOST_PWM_FREQ );
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
  TCCR1B = 0b00000100;
  TIMSK1 = ( 1 << TOIE1 );
}

ISR( TIMER1_OVF_vect ) {
  boost_pwm.refresh();
}

void loop() {
  digitalWrite( IO.pin.OUT_BOOST_PWM, HIGH );
  delayMicroseconds( boost_pwm._ton );

  noInterrupts();
  digitalWrite( IO.pin.OUT_BOOST_PWM, LOW );
  delayMicroseconds( boost_pwm._toff );
  interrupts();
}




