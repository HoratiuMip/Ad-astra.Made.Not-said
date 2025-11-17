#define RNK_KILL_LOGS_LEVEL 0x3
#include "drives/core.hpp"

#include "drives/server.hpp"

namespace rp {

TaskHandle_t h_AP_connection_light;
[[hot]]int   ap_dock_count;

void AP_connection_light( [[maybe_unused]]void* ) {
	static constexpr int PIN_NO = 33;

	pinMode( PIN_NO, OUTPUT );
for(;;) {
	digitalWrite( PIN_NO, 0x0 );
	vTaskDelay( 100 );
	digitalWrite( PIN_NO, 0x1 );
	vTaskDelay( 0x0 == ( rp::ap_dock_count = WiFi.softAPgetStationNum() ) ? 1000 : 3000 );
} }

};

rp::Eye_server EyeServer{};

void setup( void ) {
	vTaskDelay( 6000 );

	rnk::Miru.begin( rnk::miru_begin_args_t{
		tag:      rp::TAG_EYE,
		log_port: &Serial,
		log_baud: 115200	
	} );
	
	rnk::Log.info( "Booting..." );

	if( psramFound() ) {
		rnk::Log.info( "PSRAM." );
	}

	int free_dram = heap_caps_get_free_size( MALLOC_CAP_INTERNAL );
    int free_psram = heap_caps_get_free_size( MALLOC_CAP_SPIRAM );   
	rnk::Log.info( "Available DRAM[ %d ] | PSRAM[ %d ].", free_dram, free_psram );

	//rp::central_uart.begin( 115200, SERIAL_8N1, 12, 13 );

	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer   = LEDC_TIMER_0;
	config.pin_d0       = Y2_GPIO_NUM;
	config.pin_d1       = Y3_GPIO_NUM;
	config.pin_d2       = Y4_GPIO_NUM;
	config.pin_d3       = Y5_GPIO_NUM;
	config.pin_d4       = Y6_GPIO_NUM;
	config.pin_d5       = Y7_GPIO_NUM;
	config.pin_d6       = Y8_GPIO_NUM;
	config.pin_d7       = Y9_GPIO_NUM;
	config.pin_xclk     = XCLK_GPIO_NUM;
	config.pin_pclk     = PCLK_GPIO_NUM;
	config.pin_vsync    = VSYNC_GPIO_NUM;
	config.pin_href     = HREF_GPIO_NUM;
	config.pin_sccb_sda = SIOD_GPIO_NUM;
	config.pin_sccb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn     = PWDN_GPIO_NUM;
	config.pin_reset    = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.frame_size   = FRAMESIZE_VGA;
	config.pixel_format = PIXFORMAT_JPEG;
	config.grab_mode    = CAMERA_GRAB_LATEST;
	config.fb_location  = CAMERA_FB_IN_PSRAM;
	config.jpeg_quality = 10;
	config.fb_count     = 2;

	esp_err_t err = esp_camera_init( &config );
	RNK_ASSERT_OR( ESP_OK == err ) {
		rnk::Log.error( "Camera init failed with [0x%x].", err );
		rnk::Miru.after_failsafe( LED_BUILTIN_GPIO_NUM ); return;
	}

	sensor_t* sen = esp_camera_sensor_get();	 
	sen->set_brightness( sen, 0x0 );  
	sen->set_saturation( sen, 0x0 );
	
	ledcAttach( LED_GPIO_NUM, 5000, 8 );

	WiFi.persistent( false );
	WiFi.setSleep( false );

	WiFi.mode( WIFI_AP );
	WiFi.softAPConfig( { 10, 6, 15, 18 }, { 10, 6, 15, 18 }, { 255, 255, 255, 0 } );

	WiFi.softAP( rp::TAG_EYE_SSID, rp::TAG_EYE_PWD );

	EyeServer.begin( sen, &WiFi, &Serial );
	EyeServer.set_flash( 0x0 );

	xTaskCreate( &rp::AP_connection_light, "AP_connection_light", 4096, ( void* )NULL, rnk::TaskPriority_Mach, &rp::h_AP_connection_light ); 
}

void loop( void ) {
	delay(10000);
}
