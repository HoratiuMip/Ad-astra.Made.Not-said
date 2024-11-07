#pragma once

#include <wnt/common.hpp>

#include <IXT/descriptor.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <list>

namespace wnt { namespace inet_tls {


#define WNT_INET_TLS_STR WNT_STR "::inet-tls"
_WNT_IXT_COMPONENT_DESCRIPTOR( WNT_INET_TLS_STR );


enum INET_PORT : IXT::UWORD {
    INET_PORT_UNKNWN = 0,

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
    typedef   IXT::SPtr< BRIDGE >   HANDLE;

public:
    virtual const char* struct_name() const override;

public:
    BRIDGE() = default;

    BRIDGE( const BRIDGE& ) = delete;
    BRIDGE( BRIDGE&& ) = delete;

_WNT_PROTECTED:
    BRIDGE( const char* addr, INET_PORT port, IXT_COMMS_ECHO_ARG );

public:
    ~BRIDGE();

_WNT_PROTECTED:
    _SOCKET       _socket        = {};
    SSL*          _ssl           = nullptr;

    std::string   _addr          = {};
    INET_PORT     _port          = INET_PORT_UNKNWN;

    std::string   _struct_name   = WNT_INET_TLS_STR "::BRIDGE";

public:
    int write( const char* buf, int sz );
    std::string read( int sz );
    std::string xchg( const char* buf, int w_sz, int r_sz );

    static std::string pretty( const char* addr, INET_PORT port );

public:
    [[nodiscard]] static HANDLE alloc( const char* addr, INET_PORT port );
    static void free( HANDLE&& handle );

};
typedef   BRIDGE::HANDLE   HBRIDGE;


/*
| Initializes the networking and secure socket layer element.
*/
int uplink( VOID_DOUBLE_LINK vdl );

/*
| Cleans up the networking and secure socket layer element.
*/
int downlink( VOID_DOUBLE_LINK vdl );


}; };