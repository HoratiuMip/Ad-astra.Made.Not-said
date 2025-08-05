#ifndef WJPV2_CORE_HPP
#define WJPV2_CORE_HPP
/*===== Warp Joint Protocol v2 - Core - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
>
|
>=== SUPPORTED ENVIRONMENTS
> WJP_ENVIRONMENT_MINGW --- Used on Windows desktop PC.
> WJP_ENVIRONMENT_RTOS --- Used on ESP32 with RTOS.
> WJP_ENVIRONMENT_AVR --- Used on ATmega without RTOS.
|
>=== INTERNAL SEMANTICS
>_WJP_SEMANTICS_AGGREGATES_REQUIRE_CONSTRUCTOR --- Shall create constructors for aggregate types.
> _WJP_SEMANTICS_ADDRESABLE_NEW --- Shall use 'new ( <address> ) <type>{ <args...> }'.
> _WJP_SEMANTICS_STL_ATOMICS --- Shall use 'std::atomic<>' for interlocked read/writes.  
> _WJP_SEMANTICS_STL_ATOMICS_SIGNALABLE --- Shall use 'std::atomic<>::wait()/notify() for interthread signaling.
> _WJP_SEMANTICS_STL_MUTEX --- Shall use 'std::mutex'.
> _WJP_SEMANTICS_STL_FUNCTION --- Shall use 'std::function<>' instead of function pointers.
|
======*/


#if defined( WJP_ENVIRONMENT_MINGW )
    #define _WJP_forceinline __forceinline

    #define _WJP_SEMANTICS_ADDRESABLE_NEW
    
    #include <atomic>
    #define _WJP_SEMANTICS_STL_ATOMICS
    #define _WJP_SEMANTICS_STL_ATOMICS_SIGNALABLE

    #include <mutex>
    #define _WJP_SEMANTICS_STL_MUTEX

    #include <functional>
    #define _WJP_SEMANTICS_STL_FUNCTION

#elif defined( WJP_ENVIRONMENT_RTOS )
    #define _WJP_forceinline inline

    #define _WJP_SEMANTICS_ADDRESABLE_NEW

    #include <atomic>
    #define _WJP_SEMANTICS_STL_ATOMICS

    #include <mutex>
    #define _WJP_SEMANTICS_STL_MUTEX

    #include <functional>
    #define _WJP_SEMANTICS_STL_FUNCTION

#elif defined( WJP_ENVIRONMENT_AVR )
    #define _WJP_forceinline inline

    #define _WJP_SEMANTICS_AGGREGATES_REQUIRE_CONSTRUCTOR

#else
    #error "[ WJP ] Environment not specified or unsupported."
#endif 


#define _WJP_EMPTY_FUNCTION


#if defined( _WJP_SEMANTICS_STL_ATOMICS )
    #define WJP_MEM_ORD         std::memory_order
    #define WJP_MEM_ORD_ARG     std::memory_order mem_ord
    #define WJP_MEM_ORD_DFT_ARG WJP_MEM_ORD_ARG = WJP_MEM_ORD_SEQ_CST

    #define WJP_MEM_ORD_RELEASE std::memory_order_release
    #define WJP_MEM_ORD_SEQ_CST std::memory_order_seq_cst
#else
    #define WJP_MEM_ORD         char
    #define WJP_MEM_ORD_ARG     [[maybe_unused]]char mem_ord
    #define WJP_MEM_ORD_DFT_ARG [[maybe_unused]]char mem_ord = 0x0

    #define WJP_MEM_ORD_RELEASE 0x0
    #define WJP_MEM_ORD_SEQ_CST 0x0
#endif


#if defined( WJP_ARCHITECTURE_BIG ) 
    const int32_t WJP_SIG     = 0x574a5000;
    const int32_t WJP_SIG_MSK = 0xffffff00;
#else
    const int32_t WJP_SIG     = 0x00504a57;
    const int32_t WJP_SIG_MSK = 0x00ffffff;
    #if !defined( WJP_ARCHITECTURE_LITTLE )
        #warning "[ WJP ] Endianess of target architecture not specified. Defaulting to little endian."
    #endif
#endif


#define WJP_HEAD_HCTL_ALTERNATE_BIT ( 1 << 7 )
#define WJP_HEAD_HCTL_LMHI_BIT ( 1 << 6 )

#define WJP_HEAD_VCTL_ACK_REQ_BIT ( 1 << 7 )


enum WJPVerb_ : int8_t {
    WJPVerb_Null        = 0x00,

    WJPVerb_Ack         = 0x01,
    WJPVerb_Nak         = 0x02,

    WJPVerb_Heart       = 0x03,
    
    WJPVerb_QBufSet     = 0x0a,
    WJPVerb_QBufGet     = 0x0b,

    WJPVerb_IBurst      = 0x1a,
    WJPVerb_IBurstCtl   = 0x1b,

    _WJPVerb_FORCE_BYTE = 0x7f
};


template< typename _T > struct WJP_MDsc {
#if defined( _WJP_SEMANTICS_AGGREGATES_REQUIRE_CONSTRUCTOR )
    WJP_MDsc() = default;
    WJP_MDsc( _T* a, int32_t s ) : addr{ a }, sz{ s } {}
#endif

    _T*       addr   = nullptr;
    int32_t   sz     = 0;
};
typedef   WJP_MDsc< void >   WJP_MDsc_v;


#define _WJP_HEAD_HCTL_I_S_R( name, bit ) \
    _WJP_forceinline bool is_##name( void ) { return _dw1.hctl & bit; } \
    _WJP_forceinline void set_##name( void ) { _dw1.hctl |= bit; } \
    _WJP_forceinline void reset_##name( void ) { _dw1.hctl &= ~bit; }

#define _WJP_HEAD_VCTL_I_S_R( name, bit ) \
    _WJP_forceinline bool is_##name( void ) { return _dw1.vctl & bit; } \
    _WJP_forceinline void set_##name( void ) { _dw1.vctl |= bit; } \
    _WJP_forceinline void reset_##name( void ) { _dw1.vctl &= ~bit; }


struct WJP_Head {
    WJP_Head() { _dw0._sig_b0 = 0x57; _dw0._sig_b1 = 0x4a, _dw0._sig_b2 = 0x50; _dw0.verb = WJPVerb_Null; }

    union{
        struct{ int8_t _sig_b0, _sig_b1, _sig_b2; int8_t verb; } _dw0;
        int32_t                                                  sig;
    };
    struct{ uint8_t hctl = 0; uint8_t vctl = 0; int16_t noun = 0; }   _dw1;
    struct{ union{ int32_t arg = 0; int32_t arg0; }; }                _dw2;
    struct{ union{ int32_t sz = 0; int32_t arg1; }; }                 _dw3;

    _WJP_forceinline bool is_signed( void ) { return ( sig & WJP_SIG_MSK ) == WJP_SIG; }

    _WJP_HEAD_HCTL_I_S_R( alternate, WJP_HEAD_HCTL_ALTERNATE_BIT )
    _WJP_HEAD_HCTL_I_S_R( lmhi, WJP_HEAD_HCTL_LMHI_BIT )

    _WJP_HEAD_VCTL_I_S_R( ack_req, WJP_HEAD_VCTL_ACK_REQ_BIT )
};
static_assert( sizeof( WJP_Head ) == 16 );


enum WJPErr_ : int {
    /* No error. */
    WJPErr_None        = 0x0,

    /* The received header is missing the WJP signature. */
    WJPErr_NotSigned   = 0x1,

    /* Error during reception. */
    WJPErr_Recv        = 0x2,

    /* Error during transmission. */
    WJPErr_Send        = 0x3,

    /* The received packet has no resolver waiting for it. */
    WJPErr_NoResolver  = 0x4,

    /* The received packet is out of sequence. */
    WJPErr_Sequence    = 0x5,

    /* The received packet reports a wrong size. */
    WJPErr_Size        = 0x6,

    /* Either the host or the endpoint ended the connection. */
    WJPErr_ConnReset   = 0x7,

    /* The received packet can not be routed to any procedure. */
    WJPErr_DeadEnd     = 0x8,

    /* The host forced the wake of the waiting thread. */
    WJPErr_Drained     = 0x9,

    /* The receive function wrap did not read the byte count requested by WJP. */
    WJPErr_CountRecv   = 0xa,

    /* The send function wrap did not write the byte count requested by WJP. */
    WJPErr_CountSend   = 0xb,

    /* The received packet has an invalid verb code. */
    WJPErr_InvalidVerb  = 0xc,

    /* The received packet has an invalid head control bits combination. */
    WJPErr_InvalidHctl = 0xd,

    /* The received packet has an invalid operation control bits combination. */
    WJPErr_InvalidOctl = 0xe,

    /* A queue is full and the operation cannot be completed. */
    WJPErr_QueueFull   = 0xf
};
const char* WJP_err_strs[] = {
    "WJPErr_None",
    "WJPErr_NotSigned",
    "WJPErr_Recv",
    "WJPErr_Send",
    "WJPErr_NoResolver",
    "WJPErr_Sequence",
    "WJPErr_Size",
    "WJPErr_ConnReset",
    "WJPErr_NoEntry",
    "WJPErr_Forced",
    "WJPErr_CountRecv",
    "WJPErr_CountSend",
    "WJPErr_InvalidVerb",
    "WJPErr_InvalidHctl",
    "WJPErr_InvalidOctl",
    "WJPErr_QueueFull"
};


#endif