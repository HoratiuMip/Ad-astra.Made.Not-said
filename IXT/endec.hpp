#ifndef _ENGINE_ENDEC_HPP
#define _ENGINE_ENDEC_HPP
/*
*/

#include "descriptor.hpp"
#include "comms.hpp"
#include "file_manip.hpp"
#include "bit_manip.hpp"



namespace _ENGINE_NAMESPACE {



class Endec {
public:
    class Wav : public Descriptor {
    public:
        _ENGINE_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "Endec::Wav" );

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

            Unique< char[] > raw_stream{ new char[ byte_count ] };


            if( !raw_stream ) {
                echo( this, ECHO_STATUS_ERROR ) << "Bad alloc for file read buffer.";
                return;
            }


            file.read( raw_stream, byte_count );


            sample_rate = Bytes::as< unsigned int >( raw_stream + 24, 4, BIT_END_LITTLE );


            bits_per_sample = Bytes::as< unsigned short >( raw_stream + 34, 2, BIT_END_LITTLE );

            size_t bytes_per_sample = bits_per_sample / 8;

            sample_count = Bytes::as< size_t >( raw_stream + 40, 4, BIT_END_LITTLE )
                           /
                           bytes_per_sample;


            stream = new double[ sample_count ];

            ECHO_ASSERT_AND_THROW( stream, "Buffer alloc for stream." );


            double max_sample = static_cast< double >( 1 << ( bits_per_sample - 1 ) );

            for( size_t n = 0; n < sample_count; ++n )
                stream[ n ] = static_cast< double >(
                                  Bytes::as< int >( raw_stream + 44 + n * bytes_per_sample, bytes_per_sample, BIT_END_LITTLE )
                              ) / max_sample;


            channel_count = Bytes::as< unsigned short >( raw_stream + 22, 2, BIT_END_LITTLE );


            if( sample_count % channel_count != 0 )
                echo( this, ECHO_STATUS_WARNING ) << "Sample count does not distribute evenly on channel count.";

            
            sample_count /= channel_count;


            echo( this, ECHO_STATUS_OK ) << "Created from: \"" << path.data() << "\".";
        }
   
    
        Shared< double[] >   stream            = nullptr;

        size_t               sample_rate       = 0;
        size_t               bits_per_sample   = 0;
        size_t               sample_count      = 0;
        size_t               channel_count     = 0;


        static inline Wav from_file( std::string_view path, _ENGINE_ECHO_DFD_ARG ) {
            return { path, echo };
        }

    };

};



#endif