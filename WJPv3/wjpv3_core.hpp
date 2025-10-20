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


#define WJP_HEAD_CTL_PAYLOAD_BIT ( 1 << 0 )
#define WJP_HEAD_CTL_LMHI_BIT ( 1 << 1 )


enum WJPAct_ : int16_t {
    WJPVerb_Heart       = 0x00,

    _WJPVerb_FORCE_BYTE = 0x7fff
};


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
    struct{ int32_t N = 0x0; }                                        _dw3;

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
    WJPErr_Recv        = 0x2,

    /* Error during transmission. */
    WJPErr_Send        = 0x3,

    /* The received packet reports a wrong size. */
    WJPErr_N           = 0x4,

    /* The host or the endpoint ended the connection. */
    WJPErr_Reset       = 0x5,

    /* The received packet has an invalid action code. */
    WJPErr_Act         = 0x6,
   
    /* The received packet has invalid head control bits. */
    WJPErr_Ctl         = 0x7
};
const char* WJP_err_strs[] = {
    "WJPErr_None",
    "WJPErr_Align",
    "WJPErr_Recv",
    "WJPErr_Send",
    "WJPErr_N",
    "WJPErr_Reset",
    "WJPErr_Act",
    "WJPErr_Ctl"
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
    virtual int send( WJP_MDsc mdsc_ ) = 0;

    /**
     * @brief Called to receive bytes over the wire.
     * @warning This function MUST guarantee that all the requested bytes are receive, or return an error code elsewise. NO in-between.
     * @returns The count of requested bytes to receive. Negative for errors, zero for connection reset.
     */
    virtual int recv( WJP_MDsc mdsc_ ) = 0;
};


struct WJP_LMHIReceiver {
    struct Layout {
        WJP_Head*    head_in       = nullptr;
        WJP_Head*    head_out      = nullptr;
        WJP_MDsc     payload_in    = {};
        WJP_MDsc     payload_out   = {};
    };

    virtual int when_recv( Layout* layout_ ) = 0;
};


#endif