/*
*/

#include <iostream>
#include <fstream>
#include <regex>
#include <string>

#include <IXT/ring-0.hpp>
using namespace IXT;


struct CmdArgs : Descriptor {
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "CmdArgs" );

    CmdArgs( int argc, char* argv[], IXT::DWORD* result, IXT_COMMS_ECHO_ARG ) {
        static const struct Opt {
            enum C {
                C_NULL    = 0,
                C_SRC_DIR = 1,
                C_OUT_DIR = 2,
                C_MODE
            };

            const char*   name;
            int           c;
            bool          arg;
        } opts[] = {
            { name: "--source-dir", c: Opt::C_SRC_DIR, arg: true },
            { name: "--destination-dir", c: Opt::C_OUT_DIR, arg: true },
            { name: "--mode", c: Opt::C_MODE, arg: true },
            { name: "", c: Opt::C_NULL, arg: false }
        };

        *result = 1;

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

                    case Opt::C_MODE: {
                        char* end_ptr = nullptr;
                        mode = strtol( argv[ arg_idx + 1 ], &end_ptr, 0xA );

                        echo( this, ECHO_LEVEL_OK ) << "Detected conversion mode: " << ( int )mode << ".";
                    break; }
                }

                arg_idx += opt.arg;

                break;
            }
        }

        if( ins.empty() ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Empty conversion candidates list.";
            return;
        }

        if( IXT::UBYTE sit = ( src_dir.empty() << 1 ) | out_dir.empty(); sit != 0b00 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Missing " << ( ( ( sit >> 1 ) & 1 ) ? "source directory." : "output directory." );
            return;
        }

        if( mode < 0 || mode > 2 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "Invalid conversion mode.";
            return;
        }

        *result = 0;
    }

    std::string                 src_dir   = {};
    std::string                 out_dir   = {};
    std::deque< std::string >   ins       = {};
    IXT::BYTE                   mode      = -1;
};


IXT::DWORD cvt_path( const std::string& src_dir, const std::string& out_dir, IXT::BYTE mode, const std::string& rel ) {
    File::for_each_in_dir_matching( src_dir.c_str(), rel.c_str(), [ & ] ( std::string_view mch ) -> IXT::DWORD {
        auto abs_path = src_dir + '/' + mch.data();
        
        comms( ECHO_LEVEL_PENDING ) << "Matched path: \"" << abs_path.c_str() << "\".";

        std::ifstream file{ abs_path };

        if( !file ) {
            comms( ECHO_LEVEL_WARNING ) << "Could NOT open file. Continuing...";
            goto l_end;
        }
    {
        std::vector< ggfloat_t > vrtxs = {};
        ggfloat_t vrtx                 = 0.0;

        vrtxs.reserve( 64 );

        while( file >> vrtx ) vrtxs.emplace_back( vrtx ); 

        if( vrtxs.size() & 0x1 ) {
            comms( ECHO_LEVEL_WARNING ) << "Read an odd count of vertices ( " << vrtxs.size() << " ). Check the file. Continuing...";
            goto l_end;
        }


    }
    l_end:
        return FILE_FEIDM_RESULT_ITR_CONTINUE;
    } );

    return 0;
}


int main( int argc, char* argv[] ) {
    if( argc < 2 ) {
        comms( ECHO_LEVEL_ERROR ) << "No input. Aboting...";
        return 1;
    }

    IXT::DWORD result = 0;

    CmdArgs cmd_args{ argc, argv, &result };
    
    if( result != 0 ) {
        comms( ECHO_LEVEL_ERROR ) << "Command line arguments invalid or incomplete. Aborting...";
        return 1;
    }

    comms() << "Command line arguments parsed, continuing with conversions...";

    for( auto& in : cmd_args.ins ) {
        result = cvt_path( cmd_args.src_dir, cmd_args.out_dir, cmd_args.mode, in ); 
    }
}