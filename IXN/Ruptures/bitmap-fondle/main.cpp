/*
*/

#include <IXN/ring-0.hpp>
using namespace ixN;


int main( int argc, char* argv[] ) {
    if( argc != 5 ) {
        comms( EchoLevel_Error ) << "Wrong number of arguments.";
        return -1;
    }

    Endec::Bmp r{ argv[ 1 ] };
    int32_t w = r.width;
    int32_t h = r.height;

    Endec::Bmp g{ argv[ 2 ] };
    if( g.width != w || g.height != h ) {
        comms( EchoLevel_Error ) << "Bitmaps ( R, G ) widths and heights don't match.";
        return -1;
    }

    Endec::Bmp b{ argv[ 3 ] };
    if( b.width != w || b.height != h ) {
        comms( EchoLevel_Error ) << "Bitmaps ( R, B ) widths and heights don't match.";
        return -1;
    }

    for( int32_t y = 0; y < h; ++y ) {
        for( int32_t x = 0; x < w; ++x ) {
            ubyte_t& rr = *( r[ y ] + r.bytes_ps * x + 2 );
            ubyte_t  rg = *( r[ y ] + r.bytes_ps * x + 1 );
            ubyte_t  rb = *( r[ y ] + r.bytes_ps * x + 0 );

            ubyte_t bv = ( *( r[ y ] + r.bytes_ps * x + 0 ) = *( b[ y ] + b.bytes_ps * x + 0 ) );
            ubyte_t gv = ( *( r[ y ] + r.bytes_ps * x + 1 ) = *( g[ y ] + g.bytes_ps * x + 1 ) );

            if( abs( rb - bv ) > 0.1 && abs( rg - gv ) > 0.1)
                rr = 255;
            else
                rr = 0;
        }
    }

    r.write_file( argv[ 4 ] );

    comms( EchoLevel_Ok ) << "Done.";

    return 0;
}