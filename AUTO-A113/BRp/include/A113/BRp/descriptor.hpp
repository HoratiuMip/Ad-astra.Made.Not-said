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
#define A113_VERSION_STRING "AUTO-A113v1.0.0"

#define A113_inline inline
#define A113_BR_FNC
#define A113_OS_FNC

#define A113_PROTECTED protected
#define A113_PRIVATE   private

#define A113_ASSERT_OR( cond ) if( false == (cond) )


#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>


namespace A113 {


typedef   int   RESULT;


struct BUFFER {
    typedef   int   n_t;

    char*   ptr   = nullptr;
    n_t     n     = 0;    
};


};



