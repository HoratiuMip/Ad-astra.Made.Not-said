#define IXT_ECHO
#define IXT_OS_WINDOWS
#define IXT_GL_DIRECT
#define IXT_ALL_PUBLIC
#include "C:\\Hackerman\\Git\\For_Academical_Purposes\\Back\\1712301362__9T.cpp"
using namespace IXT;

#include <conio.h>



int main() {
    Audio audio{ Audio::devices()[ 0 ], 44'100, 1, 32, 256 };

    Sound snd{ audio, "sound.wav" };
    Sound snd_flt{ audio, "sound.wav" };


    Synth syn1{ audio, [] ( double x, [[ maybe_unused ]] size_t ) -> double {
        x *= 600;
        return std::sin( x ) + std::cos( 2.0 * x );
    } };
    syn1.decay_in( 10.0 );


    std::vector< double > flt{};

    for( size_t n = 1; n <= 50; ++n ) {
        flt.emplace_back( 1.0 / 25.0 * ( n % 2 ) );
    }


    for( size_t idx = 0; idx < snd.sample_count(); ++idx ) {
        int64_t hs = flt.size() / 2;
        double sum = 0.0;

        for( int64_t off = -hs; off < +hs; ++off ) {
            auto at = idx + off;

            if( at >= snd.sample_count() ) continue;

            double val = snd._stream[ at ];

            sum += flt[ off + hs ] * val;
        }

        snd_flt._stream[ idx ] = sum;
    }


    std::cout << "Ready\n";


    while( true ) {
        switch( _getch() ) {
            case 'o': snd.play(); break;
            case 'f': snd_flt.play(); break;
            case 's': snd.stop(); snd_flt.stop(); break;
            case 'h': syn1.play(); break;
        }
    }


    return 0;
}