#pragma once
/**
 * @file: OSp/IO_utils.hpp
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

class COM_Ports : public Dispenser< std::vector< COM_port_t > > {
public:
    using container_t = std::vector< COM_port_t >;

public:
    COM_Ports( const DispenserMode_ disp_mode_, bool refresh_, bool register_listen_ ) 
    : Dispenser{ disp_mode_ }
    {
        if( refresh_ ) this->refresh();
        if( register_listen_ ) this->register_listen();
    }

_A113_PROTECTED:
    HCMNOTIFICATION   _notif   = nullptr;

public:
    COM_Ports& refresh( void );

public:
    status_t register_listen( void );
    status_t unregister_listen( void );

};

}