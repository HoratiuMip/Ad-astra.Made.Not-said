#pragma once
/*===== IXN Engine - Vatca "Mipsan" Tudor-Horatiu
|
> Assert macros to separate more clearly continue ifs from other ifs.
|
======*/

#include <IXN/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



#define IXN_ASSERT( c, r ) if( !( c ) ) return r;


#define _IXN_ASSERT_ON_COMM( comm, c, r, m ) if( !( c ) ) { comm( EchoLevel_Error ) << "Assert ( " #c << " ) failed with return ( " << r << " ): " << m; return r; }
#define _IXN_ASSERT_ON_COMM_THIS( comm, c, r, m ) if( !( c ) ) { comm( this, EchoLevel_Error ) << "Assert ( " #c << " ) failed with return ( " << r << " ): " << m; return r; }

#define IXN_ASSERT_C( c, r, m ) _IXN_ASSERT_ON_COMM( comms, c, r, m )
#define IXN_ASSERT_CT( c, r, m ) _IXN_ASSERT_ON_COMM_THIS( comms, c, r, m ) 
#define IXN_ASSERT_E( c, r, m ) _IXN_ASSERT_ON_COMM( echo, c, r, m ) 
#define IXN_ASSERT_ET( c, r, m ) _IXN_ASSERT_ON_COMM_THIS( echo, c, r, m )  



};
