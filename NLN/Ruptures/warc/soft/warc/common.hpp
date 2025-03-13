#pragma once

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>

namespace warc {


#define _WARC_PROTECTED protected


#define WARC_STR "warc"

#define _WARC_NLN_COMPONENT_DESCRIPTOR( name ) \
static struct _WARC_NLN_DESCRIPTOR : public NLN::Descriptor { \
    NLN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( name ); \
} _ixt_descriptor; \

_WARC_NLN_COMPONENT_DESCRIPTOR( WARC_STR )


#define   WARC_ECHO_RT( level )             ( NLN::comms( &_ixt_descriptor, level ) )
#define   WARC_ECHO_RT_THIS( level )        ( NLN::comms( this, level ) )
#define   WARC_ECHO_RT_THAT( that, level )  ( NLN::comms( that, level ) ) 
#define   WARC_ECHO_ACC( level )            ( echo( this, level ) )

#define   WARC_ECHO_RT_OK                   WARC_ECHO_RT( NLN::EchoLevel_Ok )
#define   WARC_ECHO_RT_WARNING              WARC_ECHO_RT( NLN::EchoLevel_Warning )
#define   WARC_ECHO_RT_ERROR                WARC_ECHO_RT( NLN::EchoLevel_Error )
#define   WARC_ECHO_RT_PENDING              WARC_ECHO_RT( NLN::EchoLevel_Pending )
#define   WARC_ECHO_RT_INTEL                WARC_ECHO_RT( NLN::EchoLevel_Info )
#define   WARC_ECHO_RT_INPUT                WARC_ECHO_RT( NLN::EchoLevel_Input )
#define   WARC_ECHO_RT_DEBUG                WARC_ECHO_RT( NLN::EchoLevel_Debug )

#define   WARC_ECHO_RT_THIS_OK              WARC_ECHO_RT_THIS( NLN::EchoLevel_Ok )
#define   WARC_ECHO_RT_THIS_WARNING         WARC_ECHO_RT_THIS( NLN::EchoLevel_Warning )
#define   WARC_ECHO_RT_THIS_ERROR           WARC_ECHO_RT_THIS( NLN::EchoLevel_Error )
#define   WARC_ECHO_RT_THIS_PENDING         WARC_ECHO_RT_THIS( NLN::EchoLevel_Pending )
#define   WARC_ECHO_RT_THIS_INTEL           WARC_ECHO_RT_THIS( NLN::EchoLevel_Info )
#define   WARC_ECHO_RT_THIS_INPUT           WARC_ECHO_RT_THIS( NLN::EchoLevel_Input )
#define   WARC_ECHO_RT_THIS_DEBUG           WARC_ECHO_RT_THIS( NLN::EchoLevel_Debug )

#define   WARC_ECHO_RT_THAT_OK( that )      WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Ok )
#define   WARC_ECHO_RT_THAT_WARNING( that ) WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Warning )
#define   WARC_ECHO_RT_THAT_ERROR( that )   WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Error )
#define   WARC_ECHO_RT_THAT_PENDING( that ) WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Pending )
#define   WARC_ECHO_RT_THAT_INTEL( that )   WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Info )
#define   WARC_ECHO_RT_THAT_INPUT( that )   WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Input )
#define   WARC_ECHO_RT_THAT_DEBUG( that )   WARC_ECHO_RT_THAT( that, NLN::EchoLevel_Debug )

#define   WARC_ECHO_ACC_OK                  WARC_ECHO_ACC( NLN::EchoLevel_Ok )
#define   WARC_ECHO_ACC_WARNING             WARC_ECHO_ACC( NLN::EchoLevel_Warning )
#define   WARC_ECHO_ACC_ERROR               WARC_ECHO_ACC( NLN::EchoLevel_Error )
#define   WARC_ECHO_ACC_PENDING             WARC_ECHO_ACC( NLN::EchoLevel_Pending )
#define   WARC_ECHO_ACC_INTEL               WARC_ECHO_ACC( NLN::EchoLevel_Info )
#define   WARC_ECHO_ACC_DEBUG               WARC_ECHO_ACC( NLN::EchoLevel_Debug )


#define _WARC_ASSERT( c, m, s, r, _t ) { if( !(c) ) { ( _t ) << ( "Assert (" #c ") failed with status (" ) << ( s ) << ( "): " ) << ( m ) << ( '\n' ); return r; } }
#define WARC_ASSERT_RT( c, m, s, r ) _WARC_ASSERT( c, m, s, r, WARC_ECHO_RT_ERROR )
#define WARC_ASSERT_RT_THIS( c, m, s, r ) _WARC_ASSERT( c, m, s, r, WARC_ECHO_RT_THIS_ERROR )
#define WARC_ASSERT_ACC( c, m, s, r ) _WARC_ASSERT( c, m, s, r, WARC_ECHO_ACC_ERROR )


typedef   float   WARC_FTYPE;


inline struct _GLOBAL {
    std::filesystem::path   root_dir   = ".";
} _global;

#define WARC_ROOT_DIR ( _global.root_dir )
#define WARC_IMM_ROOT_DIR ( WARC_ROOT_DIR/"imm" )
#define WARC_DATA_ROOT_DIR ( WARC_ROOT_DIR/"data" )


};