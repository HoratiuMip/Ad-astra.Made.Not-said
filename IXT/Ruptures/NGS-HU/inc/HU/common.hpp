#pragma once

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>

namespace HU {


#define _HU_IXT_COMPONENT_DESCRIPTOR( name ) \
static struct _HU_IXT_DESCRIPTOR : public IXT::Descriptor { \
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "NGS-HU::" name ); \
} _ixt_descriptor; \

#define   HU_LOG_RT( level )    IXT::comms( &_ixt_descriptor, level )
#define   HU_LOG_ACC( level )   echo( &_ixt_descriptor, level )

#define   HU_LOG( level )       HU_LOG_RT( level )

#define   HU_LOG_OK             HU_LOG( IXT::ECHO_LEVEL_OK )
#define   HU_LOG_WARNING        HU_LOG( IXT::ECHO_LEVEL_WARNING )
#define   HU_LOG_ERROR          HU_LOG( IXT::ECHO_LEVEL_ERROR )
#define   HU_LOG_PENDING        HU_LOG( IXT::ECHO_LEVEL_PENDING )
#define   HU_LOG_INTEL          HU_LOG( IXT::ECHO_LEVEL_INTEL )


#define HU_ASSERT( c, m, r ) { if( !(c) ) { HU_LOG_ERROR << ( "Assert (" #c ") failed: " ) << m << '\n'; return r; } }


struct BLIND_ARG {
    void*    arg   = nullptr;
    void**   adr   = nullptr;
};


};