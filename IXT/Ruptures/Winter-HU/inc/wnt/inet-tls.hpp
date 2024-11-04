#pragma once

#include <wnt/common.hpp>

#include <IXT/descriptor.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <list>

namespace wnt { namespace inet_tls {


_WNT_IXT_COMPONENT_DESCRIPTOR( "inet-tls" );


enum INET_PORT : IXT::UWORD {
    INET_PORT_HTTPS = 443,

    _INET_PORT_FORCE_UWORD = 0x7f'7f
};


typedef
#if defined( WIN32 )
    ::SOCKET
#endif
_SOCKET;


class BRIDGE : public IXT::Descriptor {
public:
    friend struct _INTERNAL;

public:
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Bridge" );

public:
    BRIDGE() = default;

_WNT_PROTECTED:
    BRIDGE( const char* addr, INET_PORT port );

_WNT_PROTECTED:
    _SOCKET   _socket   = {};
    SSL*      _ssl      = nullptr;

public:
    int write( const char* buf, int sz );
    std::string read( int sz );
    std::string xchg( const char* buf, int w_sz, int r_sz );

public:
    static IXT::SPtr< BRIDGE > alloc( const char* addr, INET_PORT port );
    static void free();

};
typedef   IXT::SPtr< BRIDGE >   HBRIDGE;


/*
| Initializes the networking and secure socket layer element.
*/
int uplink( VOID_DOUBLE_LINK vdl );

/*
| Cleans up the networking and secure socket layer element.
*/
int downlink( VOID_DOUBLE_LINK vdl );


}; };