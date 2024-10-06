/*
*/
#include <IXT/endec.hpp>

using namespace IXT;

#define COUT_WIDTH std::setw( 32 )

int main() {
    Endec::Wav< double > wav{ ASSET_WAV_SAX_PATH };

    std::cout << COUT_WIDTH << "Sample count: " << wav.sample_count << '\n';
    std::cout << COUT_WIDTH << "Channel count: " << wav.tunnel_count << '\n';
    std::cout << COUT_WIDTH << "Bits per sample: " << wav.bits_per_sample << '\n';
    std::cout << COUT_WIDTH << "Sample rate: " << wav.sample_rate << '\n';

    std::cout << std::endl;

    Endec::Bmp bmp{ ASSET_BMP_AHRI_PATH };
    
    std::cout << COUT_WIDTH << "Whole buferf size: " << bmp.buf_size << '\n';
}