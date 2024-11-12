#pragma once

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

namespace warc {


#define _WARC_PROTECTED protected


#define WARC_STR "Winter-Arc"

#define _WARC_IXT_COMPONENT_DESCRIPTOR( name ) \
static struct _WARC_IXT_DESCRIPTOR : public IXT::Descriptor { \
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( name ); \
} _ixt_descriptor; \

_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_STR )


#define   WARC_LOG_RT( level )         ( IXT::comms( &_ixt_descriptor, level ) )
#define   WARC_LOG_RT_THIS( level )    ( IXT::comms( this, level ) ) 
#define   WARC_LOG_ACC( level )        ( echo( this, level ) )

#define   WARC_LOG_RT_OK               WARC_LOG_RT( IXT::ECHO_LEVEL_OK )
#define   WARC_LOG_RT_WARNING          WARC_LOG_RT( IXT::ECHO_LEVEL_WARNING )
#define   WARC_LOG_RT_ERROR            WARC_LOG_RT( IXT::ECHO_LEVEL_ERROR )
#define   WARC_LOG_RT_PENDING          WARC_LOG_RT( IXT::ECHO_LEVEL_PENDING )
#define   WARC_LOG_RT_INTEL            WARC_LOG_RT( IXT::ECHO_LEVEL_INTEL )

#define   WARC_LOG_RT_THIS_OK          WARC_LOG_RT_THIS( IXT::ECHO_LEVEL_OK )
#define   WARC_LOG_RT_THIS_WARNING     WARC_LOG_RT_THIS( IXT::ECHO_LEVEL_WARNING )
#define   WARC_LOG_RT_THIS_ERROR       WARC_LOG_RT_THIS( IXT::ECHO_LEVEL_ERROR )
#define   WARC_LOG_RT_THIS_PENDING     WARC_LOG_RT_THIS( IXT::ECHO_LEVEL_PENDING )
#define   WARC_LOG_RT_THIS_INTEL       WARC_LOG_RT_THIS( IXT::ECHO_LEVEL_INTEL )

#define   WARC_LOG_ACC_OK              WARC_LOG_ACC( IXT::ECHO_LEVEL_OK )
#define   WARC_LOG_ACC_WARNING         WARC_LOG_ACC( IXT::ECHO_LEVEL_WARNING )
#define   WARC_LOG_ACC_ERROR           WARC_LOG_ACC( IXT::ECHO_LEVEL_ERROR )
#define   WARC_LOG_ACC_PENDING         WARC_LOG_ACC( IXT::ECHO_LEVEL_PENDING )
#define   WARC_LOG_ACC_INTEL           WARC_LOG_ACC( IXT::ECHO_LEVEL_INTEL )


#define _WARC_ASSERT( c, m, s, r, _t ) { if( !(c) ) { ( _t ) << ( "Assert (" #c ") failed with status (" ) << ( s ) << ( "): " ) << ( m ) << ( '\n' ); return r; } }
#define WARC_ASSERT_RT( c, m, s, r ) _WARC_ASSERT( c, m, s, r, WARC_LOG_RT_ERROR )
#define WARC_ASSERT_RT_THIS( c, m, s, r ) _WARC_ASSERT( c, m, s, r, WARC_LOG_RT_THIS_ERROR )
#define WARC_ASSERT_ACC( c, m, s, r ) _WARC_ASSERT( c, m, s, r, WARC_LOG_ACC_ERROR )


struct VOID_DOUBLE_LINK {
    void*    flink   = nullptr;
    void**   blink   = nullptr;
};

typedef   float   WARC_FTYPE;


};