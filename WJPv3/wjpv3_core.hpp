#ifndef WJPV3_CORE_HPP
#define WJPV3_CORE_HPP
/*===== Warp Joint Protocol v3 - Core - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
>
|
======*/


#if defined( WJP_ARCHITECTURE_BIG ) 
    const int32_t WJP_SIG     = 0x574a5033;
#else
    const int32_t WJP_SIG     = 0x33504a57;
    #if !defined( WJP_ARCHITECTURE_LITTLE )
        #warning "[ WJPv3 ] Endianess of target architecture not specified. Defaulting to little endian."
    #endif
#endif


#define _WJP_forceinline inline
#define _WJP_ASSERT_OR( c ) if( !(c) )


#define WJP_HEAD_CTL_PAYLOAD_BIT ( 1 << 0 )
#define WJP_HEAD_CTL_LMHI_BIT ( 1 << 1 )


enum WJPAct_ : int16_t {
    WJPAct_Heart       = 0x00,

    _WJPAct_FORCE_BYTE = 0x7fff
};


typedef   int32_t   WJP_size_t;
typedef   int       WJP_result_t;

struct WJP_MDsc {
    char*     addr   = nullptr;
    int32_t   sz     = 0x0;
};


#define _WJP_HEAD_CTL_I_S_R( field, bit ) \
    _WJP_forceinline bool is_##field( void ) { return _dw1.CTL & bit; } \
    _WJP_forceinline void set_##field( void ) { _dw1.CTL |= bit; } \
    _WJP_forceinline void reset_##field( void ) { _dw1.CTL &= ~bit; }


struct WJP_Head {
    struct{ int32_t AGN = WJP_SIG; }                                  _dw0;
    struct{ int8_t _ = 0x0; uint8_t CTL = 0x0; int16_t ACT = 0x0; }   _dw1;
    struct{ int32_t ARG = 0x0; }                                      _dw2;
    struct{ WJP_size_t N = 0x0; }                                     _dw3;

    _WJP_forceinline bool is_aligned( void ) { return _dw0.AGN == WJP_SIG; }

    _WJP_HEAD_CTL_I_S_R( payload, WJP_HEAD_CTL_PAYLOAD_BIT )
    _WJP_HEAD_CTL_I_S_R( lmhi, WJP_HEAD_CTL_LMHI_BIT )
};
static_assert( sizeof( WJP_Head ) == 16 );


enum WJPErr_ : int {
    /* No error. */
    WJPErr_None        = 0x0,

    /* The received header is missing the WJP signature. */
    WJPErr_Align       = 0x1,

    /* Error during reception. */
    WJPErr_RX          = 0x2,

    /* Error during transmission. */
    WJPErr_TX          = 0x3,

    /* The received packet reports a wrong size. */
    WJPErr_N           = 0x4,

    /* The host or the endpoint ended the connection. */
    WJPErr_Reset       = 0x5,

    /* The received packet has an invalid action code. */
    WJPErr_Act         = 0x6,
   
    /* The received packet has invalid head control bits. */
    WJPErr_Ctl         = 0x7,

    /* An argument supplied to a routine is not valid. */
    WJPErr_RoutineArg  = 0x8
};
const char* WJP_err_strs[] = {
    "WJPErr_None",
    "WJPErr_Align",
    "WJPErr_RX",
    "WJPErr_TX",
    "WJPErr_N",
    "WJPErr_Reset",
    "WJPErr_Act",
    "WJPErr_Ctl",
    "WJPErr_RoutineArg"
};


/**
 * @brief Bridge over inter-endpoints communication.
 */
struct WJP_InterMech {
    /**
     * @brief Called to send bytes over the wire.
     * @warning This function MUST guarantee that all the requested bytes are sent, or return an error code elsewise. NO in-between.
     * @returns The count of requested bytes to send. Negative for errors, zero for connection reset.
     */
    virtual WJP_result_t WJP_mech_send( WJP_MDsc mdsc_ ) = 0;

    /**
     * @brief Called to receive bytes over the wire.
     * @warning This function MUST guarantee that all the requested bytes are receive, or return an error code elsewise. NO in-between.
     * @returns The count of requested bytes to receive. Negative for errors, zero for connection reset.
     */
    virtual WJP_result_t WJP_mech_recv( WJP_MDsc mdsc_ ) = 0;
};


struct WJP_LMHIReceiver {
    struct Layout {
        WJP_Head     head_in       = {};
        WJP_Head     head_out      = {};
        WJP_MDsc     payload_in    = {};
        WJP_MDsc     payload_out   = {};
    };

    virtual WJP_result_t WJP_lmhi_when_recv( Layout* layout_ ) = 0;
};


#endif