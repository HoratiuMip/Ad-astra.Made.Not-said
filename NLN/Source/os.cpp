/*====== IXT/NLN Engine - OS - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Implementation file.
|
======*/

#include <NLN/os.hpp>

#include <NLN/descriptor.hpp>
#include <NLN/comms.hpp>



namespace _ENGINE_NAMESPACE { namespace OS {



void SignalIntercept::_callback_proc( Signal_ code ) {
    std::unique_lock lock{ comms };
    comms.splash_critical( signal_intercept );
    comms( "Intercepted {} ({}).", _codes_strings[ code ], ( int )code );

    if( signal_intercept._callbacks.empty() ) {
        comms( " No callbacks to execute." );
        goto l_wait_input;
    }

    comms( " Executing {} callback{}...\n", signal_intercept._callbacks.size(), signal_intercept._callbacks.size() == 1 ? "" : "s" );
    lock.unlock();

    for( auto& [ _, callback ] : signal_intercept._callbacks )
        std::invoke( callback, code );

    lock.lock();
    comms.splash_critical( signal_intercept );
    comms( "\nCallbacks executed for {} ({}).", _codes_strings[ code ], ( int )code );


l_wait_input:
    comms( " Press [ ENTER ] to continue... Beware, continuing might crash the program. Read stdout first.\n\n" );
    lock.unlock();
    std::string s; std::getline( std::cin, s );
}



} };
