#pragma once

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

namespace wnt {


#define _WNT_PROTECTED protected


#define WNT_STR "Winter-HU"

#define _WNT_IXT_COMPONENT_DESCRIPTOR( name ) \
static struct _WNT_IXT_DESCRIPTOR : public IXT::Descriptor { \
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( name ); \
} _ixt_descriptor; \

#define   WNT_LOG_RT( level )    ( IXT::comms( &_ixt_descriptor, level ) )
#define   WNT_LOG_ACC( level )   ( echo( this, level ) )

#define   WNT_LOG_RT_OK             WNT_LOG_RT( IXT::ECHO_LEVEL_OK )
#define   WNT_LOG_RT_WARNING        WNT_LOG_RT( IXT::ECHO_LEVEL_WARNING )
#define   WNT_LOG_RT_ERROR          WNT_LOG_RT( IXT::ECHO_LEVEL_ERROR )
#define   WNT_LOG_RT_PENDING        WNT_LOG_RT( IXT::ECHO_LEVEL_PENDING )
#define   WNT_LOG_RT_INTEL          WNT_LOG_RT( IXT::ECHO_LEVEL_INTEL )

#define   WNT_LOG_ACC_OK            WNT_LOG_ACC( IXT::ECHO_LEVEL_OK )
#define   WNT_LOG_ACC_WARNING       WNT_LOG_ACC( IXT::ECHO_LEVEL_WARNING )
#define   WNT_LOG_ACC_ERROR         WNT_LOG_ACC( IXT::ECHO_LEVEL_ERROR )
#define   WNT_LOG_ACC_PENDING       WNT_LOG_ACC( IXT::ECHO_LEVEL_PENDING )
#define   WNT_LOG_ACC_INTEL         WNT_LOG_ACC( IXT::ECHO_LEVEL_INTEL )


#define _WNT_ASSERT( c, m, s, r, _t ) { if( !(c) ) { ( _t ) << ( "Assert (" #c ") failed with status (" ) << ( s ) << ( "): " ) << ( m ) << ( '\n' ); return r; } }
#define WNT_ASSERT_RT( c, m, s, r ) _WNT_ASSERT( c, m, s, r, WNT_LOG_RT_ERROR )
#define WNT_ASSERT_ACC( c, m, s, r ) _WNT_ASSERT( c, m, s, r, WNT_LOG_ACC_ERROR )

struct VOID_DOUBLE_LINK {
    void*    flink   = nullptr;
    void**   blink   = nullptr;
};


};