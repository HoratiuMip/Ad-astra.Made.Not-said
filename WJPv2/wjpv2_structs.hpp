#ifndef WJPV2_STRUCTS_HPP
#define WJPV2_STRUCTS_HPP
/*===== Warp Joint Protocol v2 - Structures - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> 
|
======*/
#include "wjpv2_core.hpp"
#include "wjpv2_bridges.hpp"


struct WJP_ScopedLock {
    WJP_ScopedLock( WJP_BRIDGE_Mutex* mtx )
    : _mtx{ mtx }
    {
        this->acquire();
    }

    ~WJP_ScopedLock() {
        if( _acqd == true ) this->release();
    }

    WJP_BRIDGE_Mutex*   _mtx;
    bool                _acqd;

    _WJP_forceinline void acquire( void ) { _mtx->acquire(); _acqd = true; }
    _WJP_forceinline void release( void ) { _acqd = false; _mtx->release(); }
    _WJP_forceinline bool try_acquire( void ) { return _acqd = _mtx->try_acquire(); }
};


#endif