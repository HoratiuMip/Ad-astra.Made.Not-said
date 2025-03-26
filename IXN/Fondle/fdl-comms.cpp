/*
*/
#include <iostream>

#include <IXN/comms.hpp>

using namespace ixN;


struct X : public Descriptor {
    X() = default;

    X( int n, IXN_COMMS_ECHO_ARG ) {
        echo( this, EchoLevel_Info ) << "Entered X constructor.";
    }  
};

struct Y : public Descriptor {
    IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Y" );

    Y( int n, IXN_COMMS_ECHO_ARG ) 
    : x{ n, echo }
    {
        echo( this, EchoLevel_Info ) << "Entered Y constructor.";
    }

    X x = {};
};


int main() {
//# Set comms stream to std::cout, which is optional since std::cout is the default output stream.
    comms.stream_to( std::cout );

    
//# Echos are flushed to the comms' stream when they are destructed.
    {
        Echo{}( nullptr, EchoLevel_Info ) << "Hello there!";
        Echo{}( nullptr, EchoLevel_Pending ) << "General Kenobi.";

        Echo echo{};
        echo( nullptr, EchoLevel_Ok ) << "From good,";
        echo( nullptr, EchoLevel_Warning ) << "To acceptable,";
        echo( nullptr, EchoLevel_Error ) << "To bad.";
    }


//# Constructor cascading.
    X x{ 5 }; Y y{ 5 };

//# RT logs.
    comms() << "Hello there, " << "General Kenobi!";
    comms( EchoLevel_Ok ) << "RT comms online.";

//# Mutex on RT echo.
    struct _THREAD_STRUCT : public Descriptor {
        IXN_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Thread" );
    } THREAD_STRUCT;
    {
    std::array< std::thread, 30 > ths;
    for( int n = 0; n < ths.size(); ++n ) ths[ n ] = std::thread{ [ &THREAD_STRUCT ] ( int n ) -> void {
        comms( &THREAD_STRUCT, EchoLevel_Ok ) << "Hello there comms (" << n << ")!";
    }, n };
    for( auto& t : ths ) t.join();
    }
    {
    std::array< std::thread, 30 > ths;
    for( int n = 0; n < ths.size(); ++n ) ths[ n ] = std::thread{ [ &THREAD_STRUCT ] ( int n ) -> void {
        IXN_COMMS_ECHO_RT_ARG;
        echo( THREAD_STRUCT, EchoLevel_Ok ) << "Hello there echo (" << n << ")!";
    }, n };
    for( auto& t : ths ) t.join();
    }

//# Format
    


//# Crash flush. Comms will output active echos if a termination signal is raised.
    {
        Echo echo1{}; echo1( nullptr, EchoLevel_Info ) << "Some random info.";
        Echo echo2{}; echo2( nullptr, EchoLevel_Warning ) << "Some info hinting to why the violation might have occured.";

        std::cout << *( int* )nullptr;
    }
}