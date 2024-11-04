#pragma once

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

namespace wnt {


#define _WNT_PROTECTED protected


#define _WNT_IXT_COMPONENT_DESCRIPTOR( name ) \
static struct _WNT_IXT_DESCRIPTOR : public IXT::Descriptor { \
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Winter-HU::" name ); \
} _ixt_descriptor; \

#define   WNT_LOG_RT( level )    IXT::comms( &_ixt_descriptor, level )
#define   WNT_LOG_ACC( level )   echo( &_ixt_descriptor, level )

#define   WNT_LOG( level )       WNT_LOG_RT( level )

#define   WNT_LOG_OK             WNT_LOG( IXT::ECHO_LEVEL_OK )
#define   WNT_LOG_WARNING        WNT_LOG( IXT::ECHO_LEVEL_WARNING )
#define   WNT_LOG_ERROR          WNT_LOG( IXT::ECHO_LEVEL_ERROR )
#define   WNT_LOG_PENDING        WNT_LOG( IXT::ECHO_LEVEL_PENDING )
#define   WNT_LOG_INTEL          WNT_LOG( IXT::ECHO_LEVEL_INTEL )


#define WNT_ASSERT( c, m, s, r ) { if( !(c) ) { ( WNT_LOG_ERROR ) << ( "Assert (" #c ") failed with status (" ) << ( s ) << ( "): " ) << ( m ) << ( '\n' ); return r; } }


struct VOID_DOUBLE_LINK {
    void*    flink   = nullptr;
    void**   blink   = nullptr;
};


};