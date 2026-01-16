/*
[A113] CAUTION!
THIS FILE WAS GENERATED DURING BUILD AND IT WILL BE OVERRIDEN IN THE NEXT ONE.
DO NOT MODIFY AS THE MODIFICATIONS WILL BE LOST.
*/
#pragma once
/**
 * @file: BRp/descriptor.hpp.in
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */


#define A113_VERSION_MAJOR 1
#define A113_VERSION_MINOR 0
#define A113_VERSION_PATCH 0
#define A113_VERSION_STRING "auto-a113v1.0.0"

#define A113_inline inline
#define A113_IMPL_FNC

#define _A113_PROTECTED protected
#define _A113_PRIVATE   private

#define A113_ASSERT_OR( cond ) if( !(cond) )

#define A113_STRUCT_HAS_OVR( obj, fnc ) ((void*)((obj).*(&fnc))!=(void*)(&fnc))


#ifndef _BV
    #define _BV(b) (0x1<<b)
#endif
#ifndef _SBV
    #define _SBV(x,b) (x|=b)
#endif
#ifndef _RBV
    #define _RBV(x,b) (x&=~b)
#endif
#ifndef _FBV
    #define _FBV(x,f,b) (f?_SBV(x,b):_RBV(x,b))
#endif


#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>


namespace a113 {


typedef   int   status_t;
#define A113_OK              0x0
#define A113_ERR_GENERAL     -0x1
#define A113_ERR_SYSCALL     -0x2
#define A113_ERR_WOULD_OVRWR -0x3

inline static const char* const A113_status_msgs[] = {
    "OK",
    "GENERAL",
    "SYSCALL",
    "WOULD_OVRWR"
};
#define A113_STATUS_MSG( s ) (A113_status_msgs[-(s)])

struct MDsc {
    typedef   size_t   n_t;

    char*   ptr   = nullptr;
    n_t     n     = 0;    
};


};

