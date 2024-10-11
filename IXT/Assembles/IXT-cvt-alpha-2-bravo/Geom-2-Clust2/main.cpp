/*
*/

#include <iostream>
#include <fstream>
#include <string>

#include <IXT/ring-0.hpp>
using namespace IXT;


struct CmdArgs : Descriptor {
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "CmdArgs" );

    CmdArgs( int argc, char* argv[], IXT_COMMS_ECHO_ARG ) {
        static const struct Opt {
            enum C {
                C_NULL    = 0,
                C_SRC_DIR = 1,
                C_OUT_DIR = 2
            };

            const char*   name;
            int           c;
            bool          arg;
        } opts[] = {
            { name: "--source-dir", c: Opt::C_SRC_DIR, arg: true },
            { name: "--output-dir", c: Opt::C_OUT_DIR, arg: true },
            { name: "", c: Opt::C_NULL, arg: false }
        };

        for( int arg_idx = 1; arg_idx < argc; ++arg_idx ) {
            if( !std::string_view{ argv[ arg_idx ] }.starts_with( "--" ) ) {
                ins.emplace_back( argv[ arg_idx ] );
                echo( this, ECHO_LEVEL_OK ) << "Detected conversion candidate: \"" << ins.back() << "\".";
                continue;
            }

            for( auto& opt : opts ) {
                if( strcmp( argv[ arg_idx ], opt.name ) != 0 )
                    continue;

                if( arg_idx + opt.arg == argc ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "Missing argument for: " << opt.name << ".";
                    return;
                }

                switch( opt.c ) {
                    case Opt::C_SRC_DIR: {
                        src_dir = argv[ arg_idx + 1 ];
                        echo( this, ECHO_LEVEL_OK ) << "Detected source directory: \"" << src_dir << "\".";
                    break; }

                    case Opt::C_OUT_DIR: {
                        out_dir = argv[ arg_idx + 1 ];
                        echo( this, ECHO_LEVEL_OK ) << "Detected output directory: \"" << out_dir << "\".";
                    break; }
                }

                arg_idx += opt.arg;

                break;
            }
        }

        if( ins.empty() ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Empty candidate list.";
            return;
        }

        if( ubyte_t sit = ( src_dir.empty() << 1 ) | out_dir.empty(); sit != 0b00 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Missing " << ( ( ( sit >> 1 ) & 1 ) ? "source directory." : "output directory." );
            return;
        }
    }

    std::string                 src_dir;
    std::string                 out_dir;
    std::deque< std::string >   ins;
};


int main( int argc, char* argv[] ) {
    IXT_COMMS_ECHO_ARG;

    if( argc < 2 ) {
        echo( nullptr, ECHO_LEVEL_ERROR ) << "No input.";
        return 1;
    }

    CmdArgs cmd_args{ argc, argv, echo };
    

}