#pragma once
/*
*/

#include <NLN/descriptor.hpp>
#include <NLN/bit-manip.hpp>
#include <NLN/comms.hpp>
#include <NLN/file-manip.hpp>
#include <NLN/hyper-vector.hpp>



namespace _ENGINE_NAMESPACE {



class Endec {
public:
    enum WAV_FMT {
        WAV_FMT_CHANNEL_COUNT_OFS = 0x16,
        WAV_FMT_CHANNEL_COUNT_SZ = 2,

        WAV_FMT_SAMPLE_RATE_OFS = 0x18,
        WAV_FMT_SAMPLE_RATE_SZ = 4,

        WAV_FMT_BITS_PER_SAMPLE_OFS = 0x22,
        WAV_FMT_BITS_PER_SAMPLE_SZ = 2,

        WAV_FMT_SAMPLE_COUNT_OFS = 0x28,
        WAV_FMT_SAMPLE_COUNT_SZ = 4,

        WAV_FMT_DATA_OFS = 0x2C
    };

    template< typename T >
    class Wav : public Descriptor {
    public:
        _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Endec::Wav<T>" );
    public:
        Wav() = default;

        Wav( std::string_view path, _ENGINE_COMMS_ECHO_ARG ) {
            using namespace std::string_literals;


            std::ifstream file{ path.data(), std::ios_base::binary };

            if( !file ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Could NOT open file: \"" << path.data() << "\".";
                return;
            }
        

            size_t byte_count = File::byte_count( file );

            UPtr< char[] > raw_stream{ new char[ byte_count ] };


            if( !raw_stream ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Bad alloc for file read buffer.";
                return;
            }


            file.read( raw_stream.get(), byte_count );


            tunnel_count = Bytes::as< WORD, WAV_FMT_CHANNEL_COUNT_SZ, BIT_END_LITTLE >( &WAV_FMT_CHANNEL_COUNT_OFS[ raw_stream.get() ] );

            sample_rate = Bytes::as< DWORD, WAV_FMT_SAMPLE_RATE_SZ, BIT_END_LITTLE >( &WAV_FMT_SAMPLE_RATE_OFS[ raw_stream.get() ] );

            bits_per_sample = Bytes::as< uint16_t, WAV_FMT_BITS_PER_SAMPLE_SZ, BIT_END_LITTLE >( &WAV_FMT_BITS_PER_SAMPLE_OFS[ raw_stream.get() ] );

            uint16_t bytes_per_sample = bits_per_sample / 8;

            sample_count = Bytes::as< uint64_t, WAV_FMT_SAMPLE_COUNT_SZ, BIT_END_LITTLE >( &WAV_FMT_SAMPLE_COUNT_OFS[ raw_stream.get() ] ) / bytes_per_sample;


            stream.vector( ( T* )malloc( sample_count * sizeof( T ) ) );

            if( !stream ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Bad alloc for stream buffer.";
                return;
            }


            if constexpr( std::is_floating_point_v< T > ) {
                T max_sample = static_cast< T >( 1 << ( bits_per_sample - 1 ) );

                for( size_t n = 0; n < sample_count; ++n )
                    stream[ n ] = static_cast< T >(
                                    Bytes::as< int, BIT_END_LITTLE >( &WAV_FMT_DATA_OFS[ raw_stream.get() ] + n * bytes_per_sample, bytes_per_sample )
                                ) / max_sample;
            } else {
                for( size_t n = 0; n < sample_count; ++n )
                    stream[ n ] = static_cast< T >(
                                    Bytes::as< int, BIT_END_LITTLE >( &WAV_FMT_DATA_OFS[ raw_stream.get() ] + n * bytes_per_sample, bytes_per_sample ) );
            }


            if( sample_count % tunnel_count != 0 )
                echo( this, ECHO_LEVEL_WARNING ) << "Sample count does not distribute evenly on channel count.";

            
            sample_count /= tunnel_count;


            echo( this, ECHO_LEVEL_OK ) << "Created from: \"" << path.data() << "\".";
        }

    
        HVEC< T[] >   stream            = nullptr;

        DWORD         sample_rate       = 0;
        uint16_t      bits_per_sample   = 0;
        uint64_t      sample_count      = 0;
        WORD          tunnel_count      = 0;

    };

public:
    enum BMP_FMT {
        BMP_FMT_FILE_SIZE_OFS = 0x2,
        BMP_FMT_FILE_SIZE_SZ = 4,

        BMP_FMT_DATA_OFS_OFS = 0xA,
        BMP_FMT_DATA_OFS_SZ = 4,

        BMP_FMT_WIDTH_OFS = 0x12,
        BMP_FMT_WIDTH_SZ = 4,
        BMP_FMT_HEIGHT_OFS = 0x16,
        BMP_FMT_HEIGHT_SZ = 4,

        BMP_FMT_BITS_PER_PIXEL_OFS = 0x1C,
        BMP_FMT_BITS_PER_PIXEL_SZ = 2
    };


    class Bmp : public Descriptor {
    public:
        _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Endec::Bmp" );

    public:
        Bmp() = default;

        Bmp( std::string_view path, _ENGINE_COMMS_ECHO_ARG ) {
            std::ifstream file{ path.data(), std::ios_base::binary };

            if( !file ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Could NOT open file: \"" << path.data() << "\".";
                return;
            }
        
            buf_size = File::byte_count( file );

            buffer.vector( ( ubyte_t* )malloc( buf_size * sizeof( ubyte_t ) ) );

            file.read( ( char* )buffer.get(), buf_size );

            udword_t in_file_reported_file_size = Bytes::as< udword_t, BMP_FMT_FILE_SIZE_SZ, BIT_END_LITTLE >( ( char* )&buffer[ BMP_FMT_FILE_SIZE_OFS ] );

            if( in_file_reported_file_size != buf_size )
                echo( this, ECHO_LEVEL_WARNING ) << "Actual file size ( " << buf_size << " ), is different from in-file reported file size ( " << in_file_reported_file_size << " ). The file may be either an unsupported format, or has been tampered with.";

            data_ofs = Bytes::as< udword_t, BMP_FMT_DATA_OFS_SZ, BIT_END_LITTLE >( ( char* )&buffer[ BMP_FMT_DATA_OFS_OFS ] );
            width    = Bytes::as< uint32_t, BMP_FMT_WIDTH_SZ, BIT_END_LITTLE >( ( char* )&buffer[ BMP_FMT_WIDTH_OFS ] );
            height   = Bytes::as< uint32_t, BMP_FMT_HEIGHT_SZ, BIT_END_LITTLE >( ( char* )&buffer[ BMP_FMT_HEIGHT_OFS ] );
            bits_ps  = Bytes::as< uint16_t, BMP_FMT_BITS_PER_PIXEL_SZ, BIT_END_LITTLE >( ( char* )&buffer[ BMP_FMT_BITS_PER_PIXEL_OFS ] );
            bytes_ps = bits_ps / 8;

            char mod = ( width * bytes_ps ) % 4;
            padding = ( 4 - mod ) * ( mod != 0 );

            echo( this, ECHO_LEVEL_OK ) 
            << "Created | W( " << width 
            << " ) | H( " << height
            << " ) | BPS( " << bytes_ps 
            << " ) | from: \"" << path.data() << "\".";
        }

        ~Bmp() {
            buf_size = 0;
            data_ofs = 0;
        }

    public:
        HVEC< ubyte_t[] >   buffer     = nullptr;
        size_t              buf_size   = 0;

        udword_t            data_ofs   = 0;

        int8_t              padding    = 0;
        int32_t             width      = 0;
        int32_t             height     = 0;

        uint16_t            bits_ps    = 0;
        uint16_t            bytes_ps   = 0;

    public:
        ubyte_t* operator [] ( size_t row ) {
            return buffer + ( ptrdiff_t )( data_ofs + row * ( width * bytes_ps + padding ) );
        }

    public:
        dword_t write_file( std::string_view path, _ENGINE_COMMS_ECHO_ARG ) {
            std::ofstream file{ path.data(), std::ios_base::binary };

            if( !file ) {
                echo( this, ECHO_LEVEL_ERROR ) << "Could NOT open file for write: \"" << path.data() << "\".";
                return 0;
            }

            file.write( ( char* )buffer.get(), buf_size );

            if( file.badbit ) {
                echo( this, ECHO_LEVEL_WARNING ) << "Bad bit set during write to: \"" << path.data() << "\".";
            }

            file.close();

            echo( this, ECHO_LEVEL_OK ) << "Wrote to: \"" << path.data() << "\".";

            return 0;
        }

    };

};



};
