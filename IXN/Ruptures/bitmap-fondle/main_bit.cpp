/*
*/

#include <IXN/comms.hpp>
#include <IXN/endec.hpp>
using namespace ixN;


int main( int argc, char* argv[] ) {
    if( argc <= 1 ) {
        comms( EchoLevel_Error ) << "Wrong number of arguments.";
        return -1;
    }

    Endec::Bmp r{ argv[ 1 ] };
    int32_t w = r.width;
    int32_t h = r.height;

    comms( EchoLevel_Warning, "W - {} | H - {}", w, h );

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
        if( count == 0 ) goto l_skip_pad;
        while( ++count != 8 ) bmp_byte <<= 1;
        res << "0x" << ( bmp_byte <= 0xF ? "0" : "" ) << bmp_byte << ", ";
    l_skip_pad:
        bmp_byte = count = 0;
        res << '\n'; std::cout << '\n';
    }

    std::ofstream{ argv[ 2 ] } << res.str();

/*


    std::ostringstream res; res << std::hex;
    std::ostringstream res2; res2 << std::hex;
    int count    = 0;
    int bmp_byte = 0;
    int bytes_per_row = 16;
    int bytes_at = 0;
    int last_bat = 0;

    std::cout << '\n';
    for( int32_t x = 0; x < w; ++x ) {
        int red = ( int )r[ h - 1 ][ x * r.bytes_ps + 2 ];
        if( red != 255 ) { comms( EchoLevel_Error, "Red pixel walk error." ); return -1; }

        int wx = x;
        int char_w = 0;
        for(;;) {
            ++wx; if( wx >= w ) break;
            if( ( int )r[ 0 ][ wx * r.bytes_ps + 2 ] >= 150 ) break;
            ++char_w;
        }
        if( char_w == 0 ) break;

        int wy = h - 1;
        int char_h = 0;
        for(;;) {
            --wy; if( wy < 0 ) break;
            if( ( int )r[ wy ][ x * r.bytes_ps + 2 ] >= 150 ) break;
            ++char_h;

            for( int cx = x + 1; cx <= x + char_w; ++cx ) {
                bool flag = r[ wy ][ cx * r.bytes_ps ] >= 150;
                std::cout << ( flag ? ( char )254 : ' ' );
              
                ++count;
                bmp_byte |= flag; 
                if( count == 8 ) {
                    res << "0x" << ( bmp_byte <= 0xF ? "0" : "" ) << bmp_byte << ", ";
                    bmp_byte = count = 0;
                    ++bytes_at;
                    if( bytes_at % bytes_per_row == 0 ) res << '\n';
                } else {
                    bmp_byte <<= 1;
                }
            }
            std::cout << "\n";
        }
        if( count == 0 ) goto l_skip_pad;
        while( ++count != 8 ) bmp_byte <<= 1;
        res << "0x" << ( bmp_byte <= 0xF ? "0" : "" ) << bmp_byte << ", ";
        ++bytes_at;
        if( bytes_at % bytes_per_row == 0 ) res << '\n';

    l_skip_pad:
        bmp_byte = count = 0;
        std::cout << "------------------\n";

        char buff[ 256 ];
        sprintf( buff, "{ bitmapOffset: %d, width: %d, height: %d, xAdvance: %d, xOffset: %d, yOffset: %d },\n", last_bat, char_w, char_h, char_w + 1, 0, 0 );
        res2 << buff;

        last_bat = bytes_at;
        x = wx - 1;
    }

    std::ofstream{ argv[ 2 ] } << res.str();
    std::ofstream{ argv[ 3 ] } << res2.str();
*/
    comms( EchoLevel_Ok ) << "Done.";

    return 0;
}