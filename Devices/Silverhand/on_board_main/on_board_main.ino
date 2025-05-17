/*====== HPCE_Gripper a.k.a "Silverhand" - Main -
|						                        - Vatca "Mipsan" Tudor-Horatiu 
|
|=== DESCRIPTION
> 
|
======*/


#define SERIAL_ENABLE 115200
#define BOARD_ESP32_DEVKIT_V1


#if defined BOARD_ESP32_DEVKIT_V1

#define PIN_RELAY_1 18
#define PIN_RELAY_2 5
#define PIN_RELAY_3 17
#define PIN_RELAY_4 16
#define PIN_RELAY_5 4
#define PIN_RELAY_6 15

#define PIN_KB_R0 19
#define PIN_KB_R1 21
#define PIN_KB_R2 22
#define PIN_KB_C0 23
#define PIN_KB_C1 32
#define PIN_KB_C2 33

#else
#error "Specify the target board."
#endif


#define WJP_ARCHITECTURE_LITTLE
#include "../../IXN/Driver/wjp_on_bths.hpp"
using namespace ixN::uC;


static struct GPIO {
	GPIO()
	: R1{ PIN_RELAY_1 }, /* Gray */
	  R2{ PIN_RELAY_2 }, /* Purple */
	  R3{ PIN_RELAY_3 }, /* Blue */
	  R4{ PIN_RELAY_4 }, /* Green */    /* ( colors of wires going from the uC to the relay shield , NOT of the wires from the relay shield to the walve )*/
	  R5{ PIN_RELAY_5 }, /* Yellow */
	  R6{ PIN_RELAY_6 }, /* Orange */

	  KB_R0{ PIN_KB_R0 }, KB_R1{ PIN_KB_R1 }, KB_R2{ PIN_KB_R2 },
	  KB_C0{ PIN_KB_C0 }, KB_C1{ PIN_KB_C1 }, KB_C2{ PIN_KB_C2 }
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
			};
			const uint8_t Rs[ 6 ];
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

	union {
		struct {
			const uint8_t KB_R0, KB_R1, KB_R2;
		};
		const uint8_t KB_Rs[ 3 ];
	};
	union {
		struct {
			const uint8_t KB_C0, KB_C1, KB_C2;
		};
		const uint8_t KB_Cs[ 3 ];
	};

	int init( void ) {
		for( uint8_t pin : Rs ) { 
			pinMode( pin, OUTPUT );
			digitalWrite( pin, HIGH );
		}

		pinMode( KB_R0, OUTPUT ); pinMode( KB_R1, OUTPUT ); pinMode( KB_R2, OUTPUT );
		pinMode( KB_C0, INPUT_PULLUP ); pinMode( KB_C1, INPUT_PULLUP ); pinMode( KB_C2, INPUT_PULLUP );
		digitalWrite( KB_R0, HIGH ); digitalWrite( KB_R1, HIGH ); digitalWrite( KB_R2, HIGH );

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
	inline static void open( uint8_t pin ) {
		digitalWrite( pin, HIGH );
	}

	inline static void close( uint8_t pin ) {
		digitalWrite( pin, LOW );
	}

	inline static bool make( uint8_t pin, bool flag ) {
		if( flag ) RELAY::close( pin ); else RELAY::open( pin );
		return flag;
	}
};


struct MODE_BTH {
	int init( void ) {
		return wjpblu.init( 0 );
	}

	WJP_on_BluetoothSerial   wjpblu   = {};

	int proc( bool* running ) {
		wjpblu.BluetoothSerial::begin( "Silverhand", true );
    while( !wjpblu.connected() )
		  wjpblu.connect( "BarrunCUDA" );
		return 0;
	}

} mode_bth;


struct kb_btn_t {
	uint8_t   down : 1  = 0;
	uint8_t   prsd : 1  = 0;
	uint8_t   rlsd : 1  = 0;
};

struct KB {
	inline static constexpr uint8_t   SIZE   = 3;

	inline static kb_btn_t   btns[ SIZE*SIZE ];

	static uint8_t scan( void ) {
		uint8_t count = 0;

		for( uint8_t row = 0; row < SIZE; ++row ) {
			digitalWrite( gpio.KB_Rs[ row ], LOW );

			for( uint8_t col = 0; col < SIZE; ++col ) {
				uint8_t idx = SIZE*row + col;

				bool was_down = btns[ idx ].down;
				btns[ idx ].down = !digitalRead( gpio.KB_Cs[ col ] );
				count += btns[ idx ].down;

				switch( ( was_down << 1 ) | btns[ idx ].down ) {
					case 0b00: [[fallthrough]];
					case 0b11: btns[ idx ].prsd = btns[ idx ].rlsd = 0; break;
					case 0b01: btns[ idx ].prsd = 1; btns[ idx ].rlsd = 0; break;
					case 0b10: btns[ idx ].prsd = 0; btns[ idx ].rlsd = 1; break;
				}
			}

			digitalWrite( gpio.KB_Rs[ row ], HIGH );
		}

		return count;
	}
};


void setup( void ) {
#if SERIAL_ENABLE > 0
	Serial.begin( SERIAL_ENABLE );
	Serial.println( "HPCE_Gripper - Silverhand" );
#endif

	pinMode( LED_BUILTIN, OUTPUT );

	if( gpio.init() != 0 ) _dead();

	//if( mode_bth.init() != 0 ) _dead();
}


void loop( void ) {
	KB::scan();

	RELAY::make( gpio.R5, KB::btns[ 3 ].down );
	RELAY::make( gpio.R6, KB::btns[ 5 ].down );

	RELAY::make( gpio.R4, KB::btns[ 4 ].down );
	RELAY::make( gpio.R3, KB::btns[ 7 ].down );

	RELAY::make( gpio.R1, KB::btns[ 6 ].down );
	RELAY::make( gpio.R2, KB::btns[ 8 ].down );

	delay( 100 );
}
