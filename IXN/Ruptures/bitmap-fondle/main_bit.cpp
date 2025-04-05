/*
*/

#include <IXN/comms.hpp>
#include <IXN/endec.hpp>
using namespace ixN;


int main( int argc, char* argv[] ) {
    if( argc != 3 ) {
        comms( EchoLevel_Error ) << "Wrong number of arguments.";
        return -1;
    }

    Endec::Bmp r{ argv[ 1 ] };
    int32_t w = r.width;
    int32_t h = r.height;

    std::ostringstream res; res << std::hex; 
    int count    = 0;
    int bmp_byte = 0;

    std::cout << '\n';
    for( int32_t y = h - 1; y >= 0; --y ) {
        for( int32_t x = 0; x < w; ++x ) {
            int val = ( int )r[ y ][ x * r.bytes_ps ];
            bmp_byte |= ( val > 100 );
            std::cout << ( ( val < 100 ) ? ' ' : (char)254 );
            if( count == 7 ) {
                res << "0x" << ( bmp_byte <= 0xF ? "0" : "" ) << bmp_byte << ", ";
                bmp_byte = count = 0;
            } else {
                bmp_byte <<= 1;
                ++count;
            }
        }
        res << '\n'; std::cout << '\n';
    }

    std::ofstream{ argv[ 2 ] } << res.str();

    comms( EchoLevel_Ok ) << "Done.";

    return 0;
}