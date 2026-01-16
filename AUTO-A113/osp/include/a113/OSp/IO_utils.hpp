#pragma once
/**
 * @file: osp/IO_utils.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>
#include <a113/OSp/dispenser.hpp>

namespace a113::io {

struct COM_port_t {
    std::string   id;
    std::string   friendly;
};

struct COM_Ports_config_t {
    bool   allow_refresh_callback_overwrite    = false;
    bool   clear_container_on_failed_refresh   = true;
};

struct COM_Ports_init_args_t {
    bool                 refresh   = true;
    bool                 listen    = false;
    COM_Ports_config_t   config    = {};
};

class COM_Ports : public Dispenser< std::vector< COM_port_t > > {
public:
    using container_t  = std::vector< COM_port_t >;
    using refresh_cb_t = std::function< void( container_t& ) >; 

public:
    COM_Ports( const DispenserMode_ disp_mode_, const COM_Ports_init_args_t& args_ ) 
    : Dispenser{ disp_mode_ }, _config{ std::move( args_.config ) }
    {
        if( args_.refresh ) this->refresh();
        if( args_.listen ) this->register_listen();
    }

_A113_PROTECTED:
    HCMNOTIFICATION                         _notif            = nullptr;
    std::map< std::string, refresh_cb_t >   _refresh_cb_map   = {};
    std::mutex                              _refresh_cb_mtx   = {};
    COM_Ports_config_t                      _config           = {};

public:
    COM_Ports& refresh( void );

public:
    status_t register_listen( void );
    status_t unregister_listen( void );

public:
    status_t register_refresh_callback( const char* key_, refresh_cb_t cb_ );
    status_t unregister_refresh_callback( const char* key_ );

};

}