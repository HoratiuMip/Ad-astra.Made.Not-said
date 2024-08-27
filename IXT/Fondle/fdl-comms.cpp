/*
*/
#include <iostream>

#include <IXT/comms.hpp>

using namespace IXT;


struct X : public Descriptor {
    X() = default;

    X( int n, IXT_COMMS_ECHO_ARG ) {
        echo( this, ECHO_STATUS_INTEL ) << "Entered X constructor.";
    }  
};

struct Y : public Descriptor {
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Y" );

    Y( int n, IXT_COMMS_ECHO_ARG ) 
    : x{ n, echo }
    {
        echo( this, ECHO_STATUS_INTEL ) << "Entered Y constructor.";
    }

    X x = {};
};


int main() {
//# Set comms stream to std::cout, which is optional since std::cout is the default output stream.
    comms.stream_to( std::cout );

    
//# Echos are flushed to the comms' stream when they are destructed.
    {
        Echo{}( nullptr, ECHO_STATUS_INTEL ) << "Hello there!";
        Echo{}( nullptr, ECHO_STATUS_PENDING ) << "General Kenobi.";

        Echo echo{};
        echo( nullptr, ECHO_STATUS_OK ) << "From good,";
        echo( nullptr, ECHO_STATUS_WARNING ) << "To acceptable,";
        echo( nullptr, ECHO_STATUS_ERROR ) << "To bad.";
    }


//# Constructor cascading.
    X x{ 5 }; Y y{ 5 };


//# Crash flush. Comms will output active echos if a termination signal is raised.
    {
        Echo echo1{}; echo1( nullptr, ECHO_STATUS_INTEL ) << "Some random info.";
        Echo echo2{}; echo2( nullptr, ECHO_STATUS_WARNING ) << "Some info hinting to why the violation might have occured.";

        std::cout << *( int* )nullptr;
    }
}