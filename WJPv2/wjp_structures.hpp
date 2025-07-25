/*===== Warp Joint Protocol v2 - Structures - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Structures and functions used internally by the protocol.
|
======*/
#include "wjp_internals.hpp"
#include "wjp_bridges.hpp"


/* Protocol. */
struct WJP_Head {
    WJP_Head() { _dw0._sig_b0 = 0x57; _dw0._sig_b1 = 0x4a, _dw0._sig_b2 = 0x50; _dw0.op = WJPVerb_Null; }

    union {
        struct{ int8_t _sig_b0, _sig_b1, _sig_b2; int8_t verb; } _dw0;
        int32_t                                                  sig;
    };
    struct{ uint8_t hctl = 0; uint8_t octl = 0; int16_t noun = 0; }   _dw1;
    struct{ int32_t arg = 0; }                                        _dw2;
    struct { int32_t sz = 0;  }                                       _dw3;

    _WJP_forceinline bool is_signed( void ) { return ( sig & WJP_SIG_MSK ) == WJP_SIG; }

    _WJP_forceinline bool is_alternate( void ) { return _dw1.hctl & WJP_HCTL_ALTERNATE_BIT; }
    _WJP_forceinline void set_alternate( void ) { _dw1.hctl |= WJP_HCTL_ALTERNATE_BIT; }
    _WJP_forceinline void reset_alternate( void ) { _dw1.hctl &= ~WJP_HCTL_ALTERNATE_BIT;}
};
static_assert( sizeof( WJP_Head ) == 4*sizeof( int32_t ) );


/* Memory descriptors. */
template< typename _T > struct WJP_MDsc {
#if defined( _WJP_SEMANTICS_AGGREGATES_REQUIRE_CONSTRUCTOR )
    WJP_MDsc() = default;
    WJP_MDsc( _T* a, int32_t s ) : addr{ a }, sz{ s } {}
#endif

    _T*       addr   = nullptr;
    int32_t   sz     = 0;
};
typedef   WJP_MDsc< void >   WJP_MDsc_v;


/* Concurrent. */
struct WJP_ScopedLock {
    WJP_ScopedLock( WJP_MUTEX* mtx )
    : _mtx{ mtx }
    {
        this->lock();
    }

    ~_WJP_ScopedLock() {
        if( _acqd == true ) this->unlock();
    }

    WJP_BRIDGE_Mutex*   _mtx;
    bool                _acqd;

    _WJP_forceinline void lock( void ) { _mtx->lock(); _acqd = true; }
    _WJP_forceinline void unlock( void ) { _acqd = false; _mtx->unlock(); }
    _WJP_forceinline bool try_lock( void ) { return _acqd = _mtx->try_lock(); }
};