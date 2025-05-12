/*====== HPCE_Gripper - Main -
|						     - Vatca "Mipsan" Tudor-Horatiu 
|
|=== DESCRIPTION
> 
|
======*/


static struct GPIO {
	GPIO()
	: R1{ 23 }, /* GRAY */
	  R2{ 1 },  /* PURPLE */
	  R3{ 3 },  /* BLUE */
	  R4{ 19 }, /* GREEN */
	  R5{ 18 }, /* YELLOW */
	  R6{ 5 },  /* ORANGE */
	  R7{ 17 }, /* RED */
	  R8{ 16 }  /* BROWN */
	{}

	union {
		union {
			struct {
				const uint8_t   R1;
				const uint8_t   R2;
				const uint8_t   R3;
				const uint8_t   R4;
				const uint8_t   R5;
				const uint8_t   R6;
				const uint8_t   R7;
				const uint8_t   R8;
			};
			const uint8_t Rs[ 8 ];
		};

		union {
			struct {
				const uint8_t   V1_12;
				const uint8_t   V1_14;
				const uint8_t   V2_12;
				const uint8_t   V2_14;
				const uint8_t   V3_12;
				const uint8_t   V3_14;
			};
			const uint8_t Vs[ 6 ];
		};
	};

	int init( void ) {
		for( uint8_t pin : Rs ) { 
			pinMode( pin, OUTPUT );
			digitalWrite( pin, HIGH );
		}
		return 0;
	}

} gpio;


void _dead( void ) {
	for(;;) { 
		digitalWrite( LED_BUILTIN, HIGH );
		vTaskDelay( 1000 );
		digitalWrite( LED_BUILTIN, LOW );
		vTaskDelay( 1000 );
	}
}


struct RELAY {
	static void open( uint8_t pin ) {
		digitalWrite( pin, HIGH );
	}

	static void close( uint8_t pin ) {
		digitalWrite( pin, LOW );
	}
};


void setup( void ) {
	pinMode( LED_BUILTIN, OUTPUT );

	if( gpio.init() != 0 ) _dead();
}

void loop( void ) {
	for( uint8_t r : gpio.Rs ) {
		RELAY::close( r );
		vTaskDelay( 1000 );
		RELAY::open( r );
	}
}
