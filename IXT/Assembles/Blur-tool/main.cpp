/*
*/

#include <IXT/ring-0.hpp>

using namespace IXT;

int main( int argc, char* argv[] ) {
    Endec::Bmp bmp{ ASSET_BMP_AHRI_PATH };

    for( int row = 0; row < bmp.height; ++row ) {
        for( int col = 0; col < bmp.width; ++col ) {
            bmp[ row ][ 3 * col ] = 0;
            bmp[ row ][ 3 * col + 1 ] = 40;
            bmp[ row ][ 3 * col + 2 ] = 0;
        }
    }

    bmp.write_back( "bmp_test.bmp" );
}