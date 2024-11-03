#pragma once

#include <HU/common.hpp>

#include <IXT/descriptor.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <list>

namespace HU { namespace inet_ssl {


_HU_IXT_COMPONENT_DESCRIPTOR( "inet-ssl" );


inline constexpr uint16_t   HTTPS_PORT   = 443; 


typedef
#if defined( WIN32 )
    ::SOCKET
#endif
SOCKET_TYPE;

struct SOCKET_ENTRY {
    SOCKET_TYPE   sock;
    SSL*          ssl;
};

typedef   std::list< SOCKET_ENTRY >::iterator   SOCKET_HANDLE;


/*
| Initializes the networking and secure socket layer element.
*/
int uplink( BLIND_ARG barg );

/*
| Cleans up the networking and secure socket layer element.
*/
int downlink( BLIND_ARG barg );


/*
| Creates a new secure client socket.
*/
SOCKET_HANDLE push_client_v4( const char* addr, uint16_t port );


/*
| Write/Read data.
*/
int write( SOCKET_HANDLE sock, const char* buf, int sz );

std::string read( SOCKET_HANDLE handle, int sz );


}; };