#pragma once
/*
*/

#include <IXT/descriptor.hpp>
#include <IXT/comms.hpp>
#include <IXT/file_manip.hpp>
#include <IXT/bit_manip.hpp>



namespace _ENGINE_NAMESPACE {



class Endec {
public:
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
                echo( this, ECHO_STATUS_ERROR ) << "Could NOT open file: \"" << path.data() << "\".";
                return;
            }
        

            size_t byte_count = File::byte_count( file );

            UPtr< char[] > raw_stream{ new char[ byte_count ] };


            if( !raw_stream ) {
                echo( this, ECHO_STATUS_ERROR ) << "Bad alloc for file read buffer.";
                return;
            }


            file.read( raw_stream.get(), byte_count );


            tunnel_count = Bytes::as< uint16_t, 2, BIT_END_LITTLE >( &22[ raw_stream.get() ] );

            sample_rate = Bytes::as< uint32_t, 4, BIT_END_LITTLE >( &24[ raw_stream.get() ] );

            bits_per_sample = Bytes::as< uint16_t, 2, BIT_END_LITTLE >( &34[ raw_stream.get() ] );

            uint16_t bytes_per_sample = bits_per_sample / 8;

            sample_count = Bytes::as< uint64_t, 4, BIT_END_LITTLE >( &40[ raw_stream.get() ] ) / bytes_per_sample;


            stream.reset( new T[ sample_count ] );

            if( !stream ) {
                echo( this, ECHO_STATUS_ERROR ) << "Bad alloc for stream buffer.";
                return;
            }


            if constexpr( std::is_floating_point_v< T > ) {
                T max_sample = static_cast< T >( 1 << ( bits_per_sample - 1 ) );

                for( size_t n = 0; n < sample_count; ++n )
                    stream[ n ] = static_cast< T >(
                                    Bytes::as< int, BIT_END_LITTLE >( &44[ raw_stream.get() ] + n * bytes_per_sample, bytes_per_sample )
                                ) / max_sample;
            } else {
                for( size_t n = 0; n < sample_count; ++n )
                    stream[ n ] = static_cast< T >(
                                    Bytes::as< int, BIT_END_LITTLE >( &44[ raw_stream.get() ] + n * bytes_per_sample, bytes_per_sample ) );
            }


            if( sample_count % tunnel_count != 0 )
                echo( this, ECHO_STATUS_WARNING ) << "Sample count does not distribute evenly on channel count.";

            
            sample_count /= tunnel_count;


            echo( this, ECHO_STATUS_OK ) << "Created from: \"" << path.data() << "\".";
        }

    
        SPtr< T[] >   stream            = nullptr;

        uint64_t      sample_rate       = 0;
        uint16_t      bits_per_sample   = 0;
        uint64_t      sample_count      = 0;
        uint16_t      tunnel_count      = 0;

    };

};



};
