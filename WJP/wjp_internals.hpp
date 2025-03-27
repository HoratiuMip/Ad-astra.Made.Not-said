/*===== Warp Joint Protocol - Internals - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Structures and functions used internally by the protocol.
|
======*/


#if defined( WJP_ENVIRONMENT_MINGW )
    #define _WJP_forceinline __forceinline
#elif defined( WJP_ENVIRONMENT_ARDUINO )
    #define _WJP_forceinline inline
#else
    #error "[ WJP ] Environment not specified."
#endif 