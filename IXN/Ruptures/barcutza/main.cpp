/* THIS WAS DONE WHILE DRUNK I DONNO TF HAPPENED HERE */

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <algorithm>

#define CARD int8_t
#define UPPER_CARD 14

CARD G_cards[ UPPER_CARD + 1 ];

void init( void ) {
    memset( &G_cards, 4, sizeof( G_cards ) );
}

float greater( CARD card ) {
    if( card == 0 ) return 0.5;

    int lower_count = 0;
    int upper_count = 0;

    for( CARD c = 2; c < card; ++c ) lower_count += G_cards[ c ];
    for( CARD c = card + 1; c <= UPPER_CARD; ++c ) upper_count += G_cards[ c ];

    float preffered = ( float )upper_count;
    float total = preffered + ( float )lower_count;

    std::cout << "Preffered: " << preffered << " | Total: " << total << "\n";

    return preffered / total;
}

int main( int argc, char* argv[] ) {
    srand( time( nullptr ) );

    init();

    for( int n = 1; n < argc; ++n ) {
        --G_cards[ atoi( argv[ n ] ) ];
    }

    std::string line;
    std::cout << ( ( ( float )rand() / ( float )RAND_MAX >= greater( 0 ) ) ? "mic\n" : "mare\n" );

    for( ;; ) {
        std::cin >> line; if( line == "end" ) return 0;

        CARD card = atoi( line.c_str() );

        std::cout << ( ( ( float )rand() / ( float )RAND_MAX >= greater( card ) ) ? "mic\n" : "mare\n" );

        --G_cards[ card ];
    }

    return 0;
}