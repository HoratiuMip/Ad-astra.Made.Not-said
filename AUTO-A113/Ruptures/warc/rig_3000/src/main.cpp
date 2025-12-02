#include "gps.hpp"
#include "modbus.hpp"

Modbus modbus{};

GPS gps{ GPS::pin_map_t{
	.Q_rx = 16_pin,
	.Q_tx = 17_pin
},
	modbus
};

void setup( void ) {
	core::begin( core::begin_args_t{
		.pin_map = core::pin_map_t{
			Q_seppuku: 2_pin
		}
	} );

  	modbus.begin();
	gps.begin();
}

void loop( void ) {
	vTaskDelay( 1000 );
}
