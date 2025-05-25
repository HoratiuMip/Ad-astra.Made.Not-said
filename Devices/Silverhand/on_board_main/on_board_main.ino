/*====== HPCE_Gripper a.k.a "Silverhand" - Main -
|						                        - Vatca "Mipsan" Tudor-Horatiu 
|
|=== DESCRIPTION
> 
|
======*/
#define SERIAL_ENABLE 115200

#define WJP_ARCHITECTURE_LITTLE
#include "../../IXN/Driver/wjp_on_bths.hpp"
using namespace ixN::uC;

#include "../../BarrunCUDA/barracuda.hpp"



#define SLVRHND_ASSERT( c, r, m ) if( !( c ) ) { return ( r ); }
#define SLVRHND_ASSERT_DEAD( c, m ) if( !( c ) ) { _dead(); }



namespace  GPIO {

#define PIN_RELAY_1 19
#define PIN_RELAY_2 5
#define PIN_RELAY_3 17
#define PIN_RELAY_4 16
#define PIN_RELAY_5 4
#define PIN_RELAY_6 15

#define PIN_KB_R0 13
#define PIN_KB_R1 12
#define PIN_KB_R2 14
#define PIN_KB_C0 27
#define PIN_KB_C1 26
#define PIN_KB_C2 25

typedef   uint8_t   pin_t;

static struct _pins_t {
	_pins_t() 
	:
	R1{ PIN_RELAY_1 },
	R2{ PIN_RELAY_2 },
	R3{ PIN_RELAY_3 },
	R4{ PIN_RELAY_4 },
	R5{ PIN_RELAY_5 },
	R6{ PIN_RELAY_6 },

	KB_R0{ PIN_KB_R0 },
	KB_R1{ PIN_KB_R1 },
	KB_R2{ PIN_KB_R2 },
	KB_C0{ PIN_KB_C0 },
	KB_C1{ PIN_KB_C1 },
	KB_C2{ PIN_KB_C2 }
	{}

	union {
		struct {
			const pin_t   R1;
			const pin_t   R2;
			const pin_t   R3;
			const pin_t   R4;
			const pin_t   R5;
			const pin_t   R6;
		};
		const pin_t Rs[ 6 ];
	};

	union {
		struct {
			const pin_t KB_R0, KB_R1, KB_R2;
		};
		const pin_t KB_Rs[ 3 ];
	};
	union {
		struct {
			const pin_t KB_C0, KB_C1, KB_C2;
		};
		const pin_t KB_Cs[ 3 ];
	};
} pins;

int init( void ) {
	pinMode( LED_BUILTIN, OUTPUT );

	for( pin_t pin : pins.Rs ) { 
		pinMode( pin, OUTPUT );
		digitalWrite( pin, HIGH );
	}

	for( pin_t pin : pins.KB_Cs ) {
		pinMode( pin, INPUT_PULLUP );
	}

	for( pin_t pin : pins.KB_Rs ) {
		pinMode( pin, OUTPUT );
		digitalWrite( pin, HIGH );
	}

	return 0;
}

};



struct _valve_t {
	_valve_t( GPIO::pin_t target ) : _target{ target } {}

	GPIO::pin_t _target;

	inline void open() { digitalWrite( _target, HIGH ); }

	inline void close() { digitalWrite( _target, LOW ); }

	inline bool make( bool flag ) {
		if( flag ) this->close(); else this->open();
		return flag;
	}
}
Valve_right{ GPIO::pins.R6 }, Valve_left{ GPIO::pins.R5 },
Valve_up{ GPIO::pins.R4 }, Valve_down{ GPIO::pins.R3 },
Valve_grip{ GPIO::pins.R2 }, Valve_release{ GPIO::pins.R1 }
;



namespace Bluetooth {

#define _STATUS_CONN_WAIT 1
#define _STATUS_CONN_OK 2

const char*              name          = "Silverhand";
WJP_on_BluetoothSerial   wjp_device    = {};
barra::dynamic_t         _dynamic      = {};
int                      _last_acq     = 0;
int                      _status       = 0;

int init( void ) {
	SLVRHND_ASSERT( wjp_device.init( 0 ) == 0, -1, "Fault at WJP init." );
	SLVRHND_ASSERT( wjp_device.begin( name ) == 0, -1, "Fault at WJP begin." );

	_last_acq = millis();
	_status = _STATUS_CONN_WAIT;

	return 0;
}

bool is_fresh( void ) {
	return millis() - _last_acq <= 1000;
}

namespace Commands {

bool right( void ) {
	return wjp_device.connected() && is_fresh() && _dynamic.samantha.x > barra::JS_HYST_LOW;
}
bool left( void ) {
	return wjp_device.connected() && is_fresh() && _dynamic.samantha.x < -barra::JS_HYST_LOW;
}
bool up( void ) {
	return wjp_device.connected() && is_fresh() && _dynamic.samantha.y > barra::JS_HYST_LOW;
}
bool down( void ) {
	return wjp_device.connected() && is_fresh() && _dynamic.samantha.y < -barra::JS_HYST_LOW;
}

};

void main( void* ) {
	for(;;) {
		if( !wjp_device.connected() ) { 
			_status = _STATUS_CONN_WAIT;
			vTaskDelay( 1000 ); 
			continue; 
		}
		_status = _STATUS_CONN_OK;

		WJP_WAIT_BACK_INFO    wb_info;
		WJP_RESOLVE_RECV_INFO recv_info;

		if(
			wjp_device.wait_back( &wb_info, WJPOp_QGet, WJP_CBUFFER{ addr: "DYNAM", sz: strlen( "DYNAM" ) + 1 }, WJP_BUFFER{ addr: &_dynamic, sz: sizeof( barra::dynamic_t ) }, WJPSendMethod_Direct )
			<=
			0
		) { goto l_reset; }

		while( wb_info.resolved.read() == false ) {
			if( wjp_device.resolve_recv( &recv_info ) <= 0 ) { goto l_reset; }
		}

		_last_acq = millis();
		vTaskDelay( 33 );
		goto l_end;

	l_reset:
		wjp_device.disconnect();
		wjp_device.end();

		wjp_device.~WJP_on_BluetoothSerial();
		new ( &wjp_device ) WJP_on_BluetoothSerial{};

		if( wjp_device.init( 0 ) != 0 ) { vTaskDelay( 1000 ); goto l_reset; }
		if( wjp_device.begin( name ) != 0 ) { vTaskDelay( 1000 ); goto l_reset; }

	l_end:
		continue;
	}
}

void status( void* ) {
	for(;;) {
		int s = _status;

		for( int n = 1; n <= s; ++n ) {
			digitalWrite( LED_BUILTIN, HIGH ); vTaskDelay( 100 ); digitalWrite( LED_BUILTIN, LOW ); vTaskDelay( 100 );
		}

		vTaskDelay( 1000 );
	}
}

};



namespace Keyboard {

struct btn_t {
	uint8_t   down : 1  = 0;
	uint8_t   prsd : 1  = 0;
	uint8_t   rlsd : 1  = 0;
};

constexpr uint8_t   _ROWS   = 3;
constexpr uint8_t   _COLS   = 3;

btn_t   btns[ _ROWS ][ _COLS ];

uint8_t scan( void ) {
	uint8_t count = 0;

	for( uint8_t row = 0; row < _ROWS; ++row ) {
		digitalWrite( GPIO::pins.KB_Rs[ row ], LOW );

		for( uint8_t col = 0; col < _COLS; ++col ) {
			btn_t& btn = btns[ row ][ col ];

			const bool was_down = btn.down;
			count += ( btn.down = !digitalRead( GPIO::pins.KB_Cs[ col ] ) );

			switch( ( was_down << 1 ) | btn.down ) {
				case 0b00: [[fallthrough]];
				case 0b11: btn.prsd = btn.rlsd = 0; break;
				case 0b01: btn.prsd = 1; btn.rlsd = 0; break;
				case 0b10: btn.prsd = 0; btn.rlsd = 1; break;
			}
		}

		digitalWrite( GPIO::pins.KB_Rs[ row ], HIGH );
	}

	return count;
}

};



namespace Parameters {

bool           emergency_stop            = false;

TaskHandle_t   handle_bluetooth_main     = NULL;
TaskHandle_t   handle_bluetooth_status   = NULL;

bool           valve_grip_direction      = false;

};

void _dead( void ) {
	portDISABLE_INTERRUPTS();
	vTaskSuspendAll();

	Bluetooth::wjp_device.end();

	for(;;) {
		digitalWrite( LED_BUILTIN, HIGH ); vTaskDelay( 1000 ); digitalWrite( LED_BUILTIN, LOW ); vTaskDelay( 1000 );
	}
}

void setup( void ) {
#if SERIAL_ENABLE > 0
	Serial.begin( SERIAL_ENABLE );
	Serial.println( "HPCE_Gripper - Silverhand" );
#endif

	pinMode( LED_BUILTIN, OUTPUT );

	SLVRHND_ASSERT_DEAD( GPIO::init() == 0, "Fault at GPIO init." );

	SLVRHND_ASSERT_DEAD( Bluetooth::init() == 0, "Fault at Bluetooth init." );

	SLVRHND_ASSERT_DEAD(
		xTaskCreate( &Bluetooth::status, "Bluetooth main", 8192, NULL, 5, &Parameters::handle_bluetooth_status ) == pdPASS,
		"Fault at Bluetooth status task create."
	);

 	SLVRHND_ASSERT_DEAD(
		xTaskCreate( &Bluetooth::main, "Bluetooth main", 8192, NULL, 5, &Parameters::handle_bluetooth_main ) == pdPASS,
		"Fault at Bluetooth main task create."
	);
}

void loop( void ) {
	uint8_t keyboard_in = Keyboard::scan() != 0;

	Parameters::valve_grip_direction ^= Keyboard::btns[ 2 ][ 0 ].prsd;

	if( !Keyboard::btns[ 2 ][ 0 ].prsd && Bluetooth::_dynamic.samantha.sw.prs ) {
		Parameters::valve_grip_direction ^= true;
		Bluetooth::_dynamic.samantha.sw.prs = 0;
	}

	Valve_right.make( 
		!Parameters::emergency_stop 
		&& 
		( Keyboard::btns[ 1 ][ 2 ].down || ( !keyboard_in && Bluetooth::Commands::right() ) ) 
	);

	Valve_left.make( 
		!Parameters::emergency_stop 
		&& 
		( Keyboard::btns[ 1 ][ 0 ].down || ( !keyboard_in && Bluetooth::Commands::left() ) ) 
	);

	Valve_up.make( 
		!Parameters::emergency_stop 
		&& 
		( Keyboard::btns[ 1 ][ 1 ].down || ( !keyboard_in && Bluetooth::Commands::up() ) ) 
	);

	Valve_down.make( 
		!Parameters::emergency_stop 
		&& 
		( Keyboard::btns[ 2 ][ 1 ].down || ( !keyboard_in && Bluetooth::Commands::down() ) ) 
	);

	Valve_grip.make( 
		!Parameters::emergency_stop 
		&& 
		Parameters::valve_grip_direction 
	);
	
	Valve_release.make( 
		!Parameters::emergency_stop 
		&& 
		!Parameters::valve_grip_direction
	);
}
