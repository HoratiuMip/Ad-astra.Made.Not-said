#pragma once
/*===== NLN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Assert macros to separate more clearly continue ifs from other ifs.
|
======*/

#include <NLN/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



#define NLN_ASSERT( c, r ) if( !( c ) ) return r;


#define _NLN_ASSERT_ON_COMM( comm, c, r, m ) if( !( c ) ) { comm( ECHO_LEVEL_ERROR ) << "Assert ( " #c << " ) failed with return ( " << r << " ): " << m; return r; }
#define _NLN_ASSERT_ON_COMM_THIS( comm, c, r, m ) if( !( c ) ) { comm( this, ECHO_LEVEL_ERROR ) << "Assert ( " #c << " ) failed with return ( " << r << " ): " << m; return r; }

#define NLN_ASSERT_C( c, r, m ) _NLN_ASSERT_ON_COMM( comms, c, r, m )
#define NLN_ASSERT_CT( c, r, m ) _NLN_ASSERT_ON_COMM_THIS( comms, c, r, m ) 
#define NLN_ASSERT_E( c, r, m ) _NLN_ASSERT_ON_COMM( echo, c, r, m ) 
#define NLN_ASSERT_ET( c, r, m ) _NLN_ASSERT_ON_COMM_THIS( echo, c, r, m )  



};
