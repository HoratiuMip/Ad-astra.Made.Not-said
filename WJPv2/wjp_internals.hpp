/*===== Warp Joint Protocol v2 - Internals - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Defines.
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


/* Environment specifics. */
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


/* Memory orders. */
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


/* Protocol specifics. */
#if defined( WJP_ARCHITECTURE_BIG ) 
    #define WJP_SIG     0x574a5000; 
    #define WJP_SIG_MSK 0xffffff00;
#else
    #define WJP_SIG     0x00504a57;
    #define WJP_SIG_MSK 0x00ffffff;
    #if !defined( WJP_ARCHITECTURE_LITTLE )
        #warning "[ WJP ] Endianess of target architecture not specified. Defaulting to little endian."
    #endif
#endif

#define WJP_HEAD_HCTL_ALTERNATE_BIT ( 1 << 7 )

enum WJPVerb_ : int8_t {
    WJPVerb_Null        = 0x00,

    WJPVerb_Ack         = 0x01,
    WJPVerb_Nak         = 0x02,

    WJPVerb_Heart       = 0x03,
    
    WJPVerb_QSet        = 0x0a,
    WJPVerb_QGet        = 0x0b,

    WJPVerb_IBurst      = 0x1a,
    WJPVerb_IBurstCtl   = 0x1b,

    _WJPVerb_FORCE_BYTE = 0x7f
};
