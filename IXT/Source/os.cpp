/*
*/

#include <IXT/os.hpp>

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>



namespace _ENGINE_NAMESPACE { namespace OS {



void SigInterceptor::_callback_proc( sig_t code ) {
    {
        Echo echo{}; 
        echo[ sig_interceptor ] << "Intercepted signal #" << code << ' ' << _codes_strs[ code ] << '.';

        if( sig_interceptor._callbacks.empty() ) {
            echo << " No callbacks to execute...";
            return;
        }
        
        echo << " Executing " << sig_interceptor._callbacks.size() << " callbacks...";
    }

    for( auto& [ _, callback ] : sig_interceptor._callbacks )
        std::invoke( callback, code );

    Echo{}[ sig_interceptor ] 
    << "Callbacks executed for signal #" << code << ' ' << _codes_strs[ code ]
    << ". Press [ ENTER ] to continue... Beware, continuing might crash the program. Read stdout first.";

    std::string s; std::getline( std::cin, s );
}



} };
