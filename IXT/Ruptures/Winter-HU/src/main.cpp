#include <wnt/inet-tls.hpp>


int main( int argc, char* argv[] ) {
    int status = -1;

    status = wnt::inet_tls::uplink( {} );
    
    auto n2yo_socket = wnt::inet_tls::BRIDGE::alloc( "158.69.117.9", wnt::inet_tls::INET_PORT_HTTPS );
  
    const char* request = 
    "GET /rest/v1/satellite/positions/25338/46.7/23.56/0/1/&apiKey= HTTP/1.1\r\nHost: api.n2yo.com\r\n\r\n"; 

    auto response = n2yo_socket->xchg( request, strlen( request ), 1000 );

    wnt::inet_tls::BRIDGE::free( std::move( n2yo_socket ) );

    std::cout << response << '\n';

    auto idx = response.find( "\r\n\r\n" ) + 2;
    auto sz = atoi( response.c_str() + idx );

    std::string json{ response.c_str() + idx };

    std::cout << json;


    status = wnt::inet_tls::downlink( {} );
    return status;
}