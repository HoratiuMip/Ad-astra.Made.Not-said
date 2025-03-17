/*====== IXT-NLN Engine - Comms - Vatca "Mipsan" Tudor-Horatiu
|
|=== DESCRIPTION
> Implementation file.
|
======*/

#include <NLN/comms.hpp>

#include <NLN/descriptor.hpp>



namespace _ENGINE_NAMESPACE {



_BackwardCompatibility_EchoOneLiner::_BackwardCompatibility_EchoOneLiner( Echo* echo, const Descriptor& desc, EchoLevel_ level, bool is_critical  ) 
: _echo{ echo }
{
    comms.lock();
    if( !is_critical ) 
        comms.splash( desc, level, echo );
    else
        comms.splash_critical( desc );
}
_BackwardCompatibility_EchoOneLiner::~_BackwardCompatibility_EchoOneLiner() {
    comms.unlock();
}



};
