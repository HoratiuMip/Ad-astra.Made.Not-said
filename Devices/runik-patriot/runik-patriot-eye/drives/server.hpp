#pragma once
/*
// THIS IS A COMPILATION, MODIFICATION AND ADAPTATION OF Espressif's ESP32-CAM WebServer example.
*/
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "core.hpp"

#if defined( ARDUINO_ARCH_ESP32 ) && defined( CONFIG_ARDUHAL_ESP_LOG )
	#include "esp32-hal-log.h"
#endif

#include "website_gz.h"

#include <functional>
namespace rp {


class Eye_server {
RNK_PROTECTED:
	sensor_t*             _cam             = NULL;
	WiFiClass*            _wifi            = NULL;
	HardwareSerial*       _central_uart    = NULL;

	httpd_handle_t        _httpd_control   = NULL;
	httpd_handle_t        _httpd_flux      = NULL;

	rnk::Atomic< bool >   _is_fluxing      = { false };

	rnk::pwm_t            _flash_led       = 0x0;

#define _STREAM_PART_BOUNDARY "123456789000000000000987654321"
	inline static const char*   _STREAM_CONTENT_TYPE   = "multipart/x-mixed-replace;boundary=" _STREAM_PART_BOUNDARY;
	inline static const char*   _STREAM_BOUNDARY       = "\r\n--" _STREAM_PART_BOUNDARY "\r\n";
	inline static const char*   _STREAM_PART           = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";
#undef _STREAM_PART_BOUNDARY

RNK_PROTECTED:
	static esp_err_t _handler_capture( httpd_req_t* req ) {
		esp_err_t res = ESP_OK;

		camera_fb_t* fb = esp_camera_fb_get();
		RNK_ASSERT_OR( NULL != fb ) {
			rnk::Log.error( "Capture failed." );
			httpd_resp_send_500( req );
			return ESP_FAIL;
		}

		httpd_resp_set_type( req, "image/jpeg" );
		httpd_resp_set_hdr( req, "Content-Disposition", "inline; filename=capture.jpg" );
		httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );

		char ts[ 32 ];
		snprintf( ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec );
		httpd_resp_set_hdr( req, "X-Timestamp", ( const char* )ts );

		res = httpd_resp_send( req, ( const char* )fb->buf, fb->len );

		esp_camera_fb_return( fb );
		return res;
	}

	static esp_err_t _handler_flux( httpd_req_t* req ) {
		camera_fb_t*   fb             = NULL;
		struct timeval _timestamp;
		esp_err_t      res            = ESP_OK;
		size_t         _jpg_buf_len   = 0;
		uint8_t*       _jpg_buf       = NULL;
		char*          part_buf[ 128 ];

		static int64_t last_frame = 0;
		if( !last_frame ) {
			last_frame = esp_timer_get_time();
		}

		res = httpd_resp_set_type( req, _STREAM_CONTENT_TYPE );
		RNK_ASSERT_OR( ESP_OK == res ) return res;

		httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
		httpd_resp_set_hdr( req, "X-Framerate", "60" );

		rnk::Log.info( "Flux begin." );
		rnk::Atomic< bool >* is_fluxing = &( ( Eye_server* )req->user_ctx )->_is_fluxing;
		is_fluxing->store( true );

		for(;;) {
			fb = esp_camera_fb_get();
			if( NULL == fb ) {
				rnk::Log.error( "Null frame buffer during flux." );
				res = ESP_FAIL;
				break;
			} else {
				_timestamp.tv_sec = fb->timestamp.tv_sec;
				_timestamp.tv_usec = fb->timestamp.tv_usec;
				_jpg_buf_len = fb->len;
				_jpg_buf = fb->buf;
			}
			if( ESP_OK == res ) res = httpd_resp_send_chunk( req, _STREAM_BOUNDARY, strlen( _STREAM_BOUNDARY ) );
			{
				res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
			}
			if( ESP_OK == res ) {
				size_t hlen = snprintf( ( char* )part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec );
				res = httpd_resp_send_chunk( req, ( const char* )part_buf, hlen );
			}
			if( ESP_OK == res ) {
				res = httpd_resp_send_chunk( req, ( const char* )_jpg_buf, _jpg_buf_len );
			}
			if( ESP_OK != res ) {
				rnk::Log.error( "Frame transmission failed." );
				break;
			}

			esp_camera_fb_return( fb );
		}

		is_fluxing->store( false );
		rnk::Log.info( "Flux end." );
		return res;
	}

	static std::unique_ptr< char[] > _parse_get( httpd_req_t* req ) {
		size_t buf_len = 0;

		buf_len = httpd_req_get_url_query_len( req ) + 1;
		RNK_ASSERT_OR( buf_len > 1 ) {
			httpd_resp_send_404( req );
			return NULL;
		}

		std::unique_ptr< char[] > buf{ new char[ buf_len ] };
		RNK_ASSERT_OR( buf ) {
			httpd_resp_send_500( req );
			return NULL;
		}

		RNK_ASSERT_OR( ESP_OK == httpd_req_get_url_query_str( req, buf.get(), buf_len ) ) {
			return NULL;
		}

		return buf;
	}

	static esp_err_t _handler_control( httpd_req_t* req ) {
		auto* eye = ( ( Eye_server* )req->user_ctx );
		auto* cam = eye->_cam;

		char  buf_variable[ 32 ];
		char  buf_value[ 32 ];

		auto buf_get = _parse_get( req );
		RNK_ASSERT_OR( buf_get ) return ESP_FAIL;
		rnk::Log.info( "%s", buf_get.get() );

		RNK_ASSERT_OR(
			ESP_OK == httpd_query_key_value( buf_get.get(), "var", buf_variable, sizeof( buf_variable ) )
			&&
			ESP_OK == httpd_query_key_value( buf_get.get(), "val", buf_value, sizeof( buf_value ) )
		) {
			httpd_resp_send_404( req );
			return ESP_FAIL;
		}

		int value = atoi( buf_value );
		rnk::Log.info( "Control: [%s]->[%d].", buf_variable, value );
		
		int res = 0;
	#define _SET_VARIABLE_MAP( var_id ) else if( NULL == strcmp( buf_variable, #var_id ) ) res = cam->set_##var_id( cam, value )
	#define _SET_VARIABLE_MAP2( var_id, fnc_id ) else if( NULL == strcmp( buf_variable, #var_id ) ) res = cam->set_##fnc_id( cam, value )
		if( NULL == strcmp( buf_variable, "led_intensity" ) ) { eye->set_flash( value );  }
		_SET_VARIABLE_MAP( quality );
		_SET_VARIABLE_MAP( contrast );
		_SET_VARIABLE_MAP( brightness );
		_SET_VARIABLE_MAP( saturation );
		_SET_VARIABLE_MAP( colorbar );
		_SET_VARIABLE_MAP2( awb, whitebal );
		_SET_VARIABLE_MAP2( agc, gain_ctrl );
		_SET_VARIABLE_MAP2( aec, exposure_ctrl );
		_SET_VARIABLE_MAP( hmirror );
		_SET_VARIABLE_MAP( vflip );
		_SET_VARIABLE_MAP( awb_gain );
		_SET_VARIABLE_MAP( agc_gain );
		_SET_VARIABLE_MAP( aec_value );
		_SET_VARIABLE_MAP( aec2 );
		_SET_VARIABLE_MAP( dcw );
		_SET_VARIABLE_MAP( bpc );
		_SET_VARIABLE_MAP( wpc );
		_SET_VARIABLE_MAP( raw_gma );
		_SET_VARIABLE_MAP( lenc );
		_SET_VARIABLE_MAP( special_effect );
		_SET_VARIABLE_MAP( wb_mode );
		_SET_VARIABLE_MAP( ae_level );
		else if( NULL == strcmp( buf_variable, "gainceiling" ) ) res = cam->set_gainceiling( cam, ( gainceiling_t )value );
		else {
			rnk::Log.error( "Unknown control: [%s].", buf_variable );
			res = -0x1;
		}

		RNK_ASSERT_OR( res >= 0x0 ) return httpd_resp_send_500( req );

		httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
		return httpd_resp_send( req, NULL, 0 );
	}

	static esp_err_t _handler_vcmd( httpd_req_t* req ) {
		auto* eye = ( ( Eye_server* )req->user_ctx );
		auto* cam = eye->_cam;

		char  buf_track_left[ 32 ];
		char  buf_track_right[ 32 ];

		auto buf_get = _parse_get( req );
		RNK_ASSERT_OR( buf_get ) return ESP_FAIL;
		rnk::Log.info( "%s", buf_get.get() );
		eye->_central_uart->println( buf_get.get() );

		httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
		return httpd_resp_send( req, NULL, 0 );
	}

	static esp_err_t _handler_status( httpd_req_t* req ) {
		auto* eye = ( ( Eye_server* )req->user_ctx );
		auto* cam = eye->_cam;
		
		static char json_response[ 1024 ];

		char *p = json_response;
		*p++ = '{';

		p += sprintf( p, "\"0x%x\":%u,", 0xd3,     cam->get_reg( cam, 0xd3, 0xFF ) );
		p += sprintf( p, "\"0x%x\":%u,", 0x111,    cam->get_reg( cam, 0x111, 0xFF ) );
		p += sprintf( p, "\"0x%x\":%u,", 0x132,    cam->get_reg( cam, 0x132, 0xFF ) );
		p += sprintf( p, "\"xclk\":%u,",           cam->xclk_freq_hz / 1000000 );
		p += sprintf( p, "\"pixformat\":%u,",      cam->pixformat );
		p += sprintf( p, "\"framesize\":%u,",      cam->status.framesize );
		p += sprintf( p, "\"quality\":%u,",        cam->status.quality );
		p += sprintf( p, "\"brightness\":%d,",     cam->status.brightness );
		p += sprintf( p, "\"contrast\":%d,",       cam->status.contrast );
		p += sprintf( p, "\"saturation\":%d,",     cam->status.saturation );
		p += sprintf( p, "\"sharpness\":%d,",      cam->status.sharpness );
		p += sprintf( p, "\"special_effect\":%u,", cam->status.special_effect );
		p += sprintf( p, "\"wb_mode\":%u,",        cam->status.wb_mode );
		p += sprintf( p, "\"awb\":%u,",            cam->status.awb );
		p += sprintf( p, "\"awb_gain\":%u,",       cam->status.awb_gain );
		p += sprintf( p, "\"aec\":%u,",            cam->status.aec );
		p += sprintf( p, "\"aec2\":%u,",           cam->status.aec2 );
		p += sprintf( p, "\"ae_level\":%d,",       cam->status.ae_level );
		p += sprintf( p, "\"aec_value\":%u,",      cam->status.aec_value );
		p += sprintf( p, "\"agc\":%u,",            cam->status.agc );
		p += sprintf( p, "\"agc_gain\":%u,",       cam->status.agc_gain );
		p += sprintf( p, "\"gainceiling\":%u,",    cam->status.gainceiling );
		p += sprintf( p, "\"bpc\":%u,",            cam->status.bpc );
		p += sprintf( p, "\"wpc\":%u,",            cam->status.wpc );
		p += sprintf( p, "\"raw_gma\":%u,",        cam->status.raw_gma );
		p += sprintf( p, "\"lenc\":%u,",           cam->status.lenc );
		p += sprintf( p, "\"hmirror\":%u,",        cam->status.hmirror );
		p += sprintf( p, "\"vflip\":%u,",          cam->status.vflip );
		p += sprintf( p, "\"dcw\":%u,",            cam->status.dcw );
		p += sprintf( p, "\"colorbar\":%u",        cam->status.colorbar );
		p += sprintf( p, ",\"led_intensity\":%u",  eye->_flash_led );
		*p++ = '}';
		*p++ = 0;

		httpd_resp_set_type( req, "application/json" );
		httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
		return httpd_resp_send( req, json_response, strlen( json_response ) );
	}

	static esp_err_t _handler_website( httpd_req_t* req ) {
		auto* eye = ( ( Eye_server* )req->user_ctx );
		auto* cam = eye->_cam;

		httpd_resp_set_type( req, "text/html" );
		httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
		
		if( NULL != cam )
			return httpd_resp_send( req, (const char *)RP_website_gz, RP_website_gz_size );

		return httpd_resp_send_500( req );
	}

public:
	rnk::status_t begin( sensor_t* cam_, WiFiClass* wifi_, HardwareSerial* central_uart_ ) {
		httpd_config_t config = HTTPD_DEFAULT_CONFIG();
		config.max_uri_handlers = 16;
		config.stack_size = 8192;

		httpd_uri_t uri_website = {
			.uri = "/",
			.method = HTTP_GET,
			.handler = &Eye_server::_handler_website,
			.user_ctx = ( void* )this
	#ifdef CONFIG_HTTPD_WS_SUPPORT
			,
			.is_websocket = true,
			.handle_ws_control_frames = false,
			.supported_subprotocol = NULL
	#endif
		};

		httpd_uri_t uri_status = {
			.uri = "/status",
			.method = HTTP_GET,
			.handler = &Eye_server::_handler_status,
			.user_ctx = ( void* )this
	#ifdef CONFIG_HTTPD_WS_SUPPORT
			,
			.is_websocket = true,
			.handle_ws_control_frames = false,
			.supported_subprotocol = NULL
	#endif
		};

		httpd_uri_t uri_control = {
			.uri = "/control",
			.method = HTTP_GET,
			.handler = &Eye_server::_handler_control,
			.user_ctx = ( void* )this
	#ifdef CONFIG_HTTPD_WS_SUPPORT
			,
			.is_websocket = true,
			.handle_ws_control_frames = false,
			.supported_subprotocol = NULL
	#endif
		};

		httpd_uri_t capture_uri = {
			.uri = "/capture",
			.method = HTTP_GET,
			.handler = _handler_capture,
			.user_ctx = ( void* )this
	#ifdef CONFIG_HTTPD_WS_SUPPORT
			,
			.is_websocket = true,
			.handle_ws_control_frames = false,
			.supported_subprotocol = NULL
	#endif
		};

		httpd_uri_t uri_flux = {
			.uri = "/flux",
			.method = HTTP_GET,
			.handler = &Eye_server::_handler_flux,
			.user_ctx = ( void* )this
	#ifdef CONFIG_HTTPD_WS_SUPPORT
			,
			.is_websocket = true,
			.handle_ws_control_frames = false,
			.supported_subprotocol = NULL
	#endif
		};

		httpd_uri_t uri_vcmd = {
			.uri = "/vcmd",
			.method = HTTP_GET,
			.handler = &Eye_server::_handler_vcmd,
			.user_ctx = ( void* )this
	#ifdef CONFIG_HTTPD_WS_SUPPORT
			,
			.is_websocket = true,
			.handle_ws_control_frames = false,
			.supported_subprotocol = NULL
	#endif
		};

		rnk::Log.info( "Control server on port: [%d]...", config.server_port );
		RNK_ASSERT_OR( ESP_OK == httpd_start( &_httpd_control, &config ) ) {
			rnk::Log.error( "Failed to start control server." );
			return -0x1;
		}

		httpd_register_uri_handler( _httpd_control, &uri_website );
		httpd_register_uri_handler( _httpd_control, &uri_control );
		httpd_register_uri_handler( _httpd_control, &uri_status );
		httpd_register_uri_handler( _httpd_control, &uri_vcmd );
		httpd_register_uri_handler( _httpd_control, &capture_uri );

		config.server_port += 1;
		config.ctrl_port += 1;
		rnk::Log.info( "Flux server on port: [%d]...", config.server_port );
		RNK_ASSERT_OR( ESP_OK == httpd_start( &_httpd_flux, &config ) ) {
			rnk::Log.error( "Failed to start flux server." );
			return -0x1;
		}

		httpd_register_uri_handler( _httpd_flux, &uri_flux );

		_cam          = cam_;
		_wifi         = wifi_;
		_central_uart = central_uart_;

		return 0x0;
	}

public:
	RNK_inline void set_flash( rnk::pwm_t pwm_ ) {
		ledcWrite( LED_GPIO_NUM, _flash_led = pwm_ ); 
	}

};


};
