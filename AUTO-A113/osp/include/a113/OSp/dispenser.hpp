#pragma once
/**
 * @file: OSp/dispenser.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>

namespace a113 {


enum DispenserMode_ {
    DispenserMode_Lock, DispenserMode_Swap
};

template< typename _T_ > class Dispenser : public HVec< _T_ >, public std::shared_mutex {
public:
    template< typename > friend struct dispenser_acquire;

public:
    Dispenser( const DispenserMode_ mode_ ) : _mode{ mode_ } {}

_A113_PROTECTED:
    const DispenserMode_   _mode;

public:
    A113_inline const DispenserMode_ disp_mode( void ) const { return _mode; }

};

template< typename _T_ > struct dispenser_acquire {
public:
    dispenser_acquire( Dispenser< _T_ >& disp_ ) : _disp{ disp_ } {
        if( _disp._mode == DispenserMode_Lock ) { _disp.lock_shared(); } 
        else { _ref = _disp; }
    }

    ~dispenser_acquire( void ) {
        if( _disp._mode == DispenserMode_Lock ) { _disp.unlock_shared(); } 
    }

_A113_PROTECTED:
    Dispenser< _T_ >&   _disp;
    HVec< _T_ >         _ref    = {};

public:
    A113_inline _T_* operator -> ( void ) {
        if( _disp._mode == DispenserMode_Lock ) { return _disp.get(); } else { return _ref.get(); }
    }

    A113_inline _T_& operator * ( void ) {
        if( _disp._mode == DispenserMode_Lock ) { return *_disp.get(); } else { return *_ref.get(); }
    }

};


}