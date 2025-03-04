/*
*/

#include <NLN/os.hpp>

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>



namespace _ENGINE_NAMESPACE { namespace OS {



void SigInterceptor::_callback_proc( sig_t code ) {
    {
        Echo echo{}; 
        echo[ sig_interceptor ] << "Intercepted signal #" << code << ' ' << _codes_strs[ code ] << '.';

        if( sig_interceptor._callbacks.empty() ) {
            echo << " No callbacks to execute...";
            return;
        }

        size_t callbacks_count = sig_interceptor._callbacks.size();
        
        echo << " Executing " << callbacks_count << " callback" << ( callbacks_count == 1 ? "" : "s" ) << "...";
    }

    for( auto& [ _, callback ] : sig_interceptor._callbacks )
        std::invoke( callback, code );

    Echo{}[ sig_interceptor ] 
    << "Callbacks executed for signal #" << code << ' ' << _codes_strs[ code ]
    << ". Press [ ENTER ] to continue... Beware, continuing might crash the program. Read stdout first.";

    std::string s; std::getline( std::cin, s );
}



} };
