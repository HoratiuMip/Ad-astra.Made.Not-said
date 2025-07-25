/*===== Warp Joint Protocol v2 - Bridges - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Interfaces for structures used inside the protocol.
|
======*/
#include "wjp_internals.hpp"


/* Concurrent. */
template< typename _T > struct WJP_BRIDGE_Interlocked {
    virtual _T read( WJP_MEM_ORD_DFT_ARG ) = 0;
    virtual void write( _T flag, WJP_MEM_ORD_DFT_ARG ) = 0;

    virtual void sig_one( void ) = 0;
    virtual void sig_all( void ) = 0;

    virtual void wait( _T flag ) = 0;
};

struct WJP_BRIDGE_Mutex {
    virtual void lock( void ) = 0;
    virtual void unlock( void ) = 0;
    virtual bool try_lock( void ) = 0;
};


/* Containers. */
template< typename _T > struct WJP_BRIDGE_Queue {
    virtual bool is_empty( void ) const = 0;

    virtual _T* push( _T&& arg ) = 0;
    virtual _T* pop( void ) = 0;
    virtual int clear( void ) = 0;

    virtual _T* front( void ) = 0;
    virtual void for_each( for_eache_cb_t cb ) = 0;
};
