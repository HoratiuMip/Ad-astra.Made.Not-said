/*
[ DESCRIPTION ]
    |>> Example of Endec usage.
*/
#include <IXT/endec.hpp>

using namespace IXT;



int main() {
    Endec::Wav< double > wav{ "Assets/song.wav" };

    std::cout << std::setw( 20 ) << "Sample count: " << wav.sample_count << '\n';
    std::cout << std::setw( 20 ) << "Channel count: " << wav.channel_count << '\n';
    std::cout << std::setw( 20 ) << "Bits per sample: " << wav.bits_per_sample << '\n';
    std::cout << std::setw( 20 ) << "Sample rate: " << wav.sample_rate << '\n';
}