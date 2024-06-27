#ifndef _ENGINE_OS_CPP
#define _ENGINE_OS_CPP



#include "os.hpp"

#include "descriptor.hpp"
#include "comms.hpp"



namespace _ENGINE_NAMESPACE { namespace OS {



void SigInterceptor::_callback_proc( SIG code ) {
    Echo{}[ sig_interceptor ] 
    << "Intercepted signal #" << code << ' ' << _codes_strs[ code ]
    << ". Executing callbacks...";

    for( auto& [ _, callback ] : sig_interceptor._callbacks )
        std::invoke( callback, code );

    Echo{}[ sig_interceptor ] 
    << "Callbacks executed for signal #" << code << ' ' << _codes_strs[ code ]
    << ". Press [ ENTER ] to continue... Beware, continuing might crash the program. Read stdout first.";

    std::string s; std::getline( std::cin, s );
}



} };



#endif
