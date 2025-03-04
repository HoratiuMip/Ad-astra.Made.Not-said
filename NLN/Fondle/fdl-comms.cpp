/*
*/
#include <iostream>

#include <NLN/comms.hpp>

using namespace NLN;


struct X : public Descriptor {
    X() = default;

    X( int n, NLN_COMMS_ECHO_ARG ) {
        echo( this, ECHO_LEVEL_INTEL ) << "Entered X constructor.";
    }  
};

struct Y : public Descriptor {
    NLN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Y" );

    Y( int n, NLN_COMMS_ECHO_ARG ) 
    : x{ n, echo }
    {
        echo( this, ECHO_LEVEL_INTEL ) << "Entered Y constructor.";
    }

    X x = {};
};


int main() {
//# Set comms stream to std::cout, which is optional since std::cout is the default output stream.
    comms.stream_to( std::cout );

    
//# Echos are flushed to the comms' stream when they are destructed.
    {
        Echo{}( nullptr, ECHO_LEVEL_INTEL ) << "Hello there!";
        Echo{}( nullptr, ECHO_LEVEL_PENDING ) << "General Kenobi.";

        Echo echo{};
        echo( nullptr, ECHO_LEVEL_OK ) << "From good,";
        echo( nullptr, ECHO_LEVEL_WARNING ) << "To acceptable,";
        echo( nullptr, ECHO_LEVEL_ERROR ) << "To bad.";
    }


//# Constructor cascading.
    X x{ 5 }; Y y{ 5 };

//# RT logs.
    comms() << "Hello there, " << "General Kenobi!";
    comms( ECHO_LEVEL_OK ) << "RT comms online.";

//# Crash flush. Comms will output active echos if a termination signal is raised.
    {
        Echo echo1{}; echo1( nullptr, ECHO_LEVEL_INTEL ) << "Some random info.";
        Echo echo2{}; echo2( nullptr, ECHO_LEVEL_WARNING ) << "Some info hinting to why the violation might have occured.";

        std::cout << *( int* )nullptr;
    }
}