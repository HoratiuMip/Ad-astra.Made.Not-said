#pragma once

#include <warc/common.hpp>

#include <IXT/descriptor.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <list>
#include <mutex>


namespace warc { namespace inet_tls {


#define WARC_INET_TLS_STR WARC_STR"::inet-tls"
_WARC_IXT_COMPONENT_DESCRIPTOR( WARC_INET_TLS_STR );


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

_WARC_PROTECTED:
    BRIDGE( const char* addr, INET_PORT port );

public:
    ~BRIDGE();

_WARC_PROTECTED:
    _SOCKET       _socket        = {};
    SSL*          _ssl           = nullptr;

    std::string   _addr          = {};
    INET_PORT     _port          = INET_PORT_UNKNWN;

    std::string   _struct_name   = WARC_INET_TLS_STR"::BRIDGE";

_WARC_PROTECTED:
    int _kill_conn();

public:
    int write( const char* buf, int sz );
    std::string read( int sz );
    std::string xchg( const char* buf, int w_sz, int r_sz );

    static std::string pretty( const char* addr, INET_PORT port );

public:
    [[nodiscard]] static HANDLE alloc( const char* addr, INET_PORT port );
    static void free( HANDLE& handle );

public:
    bool usable();

};
typedef   BRIDGE::HANDLE   HBRIDGE;


/*
| Initializes the networking and secure socket layer element.
*/
int uplink();

/*
| Cleans up the networking and secure socket layer element.
*/
int downlink();


}; };