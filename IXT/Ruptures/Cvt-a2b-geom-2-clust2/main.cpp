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
                C_MODE    = 3,
                C_NORM    = 4
            };

            const char*   name;
            int           c;
            bool          arg;
        } opts[] = {
            { name: "--source-dir", c: Opt::C_SRC_DIR, arg: true },
            { name: "--destination-dir", c: Opt::C_OUT_DIR, arg: true },
            { name: "--mode", c: Opt::C_MODE, arg: true },
            { name: "--normalize", c: Opt::C_NORM, arg:false },
            { name: "", c: Opt::C_NULL, arg: false }
        };

        *result = 1;

        for( int arg_idx = 1; arg_idx < argc; ++arg_idx ) {
            if( !std::string_view{ argv[ arg_idx ] }.starts_with( "--" ) ) {
                ins.emplace_back( argv[ arg_idx ] );
                echo( this, ECHO_LEVEL_OK ) << "Detected cvt regex: \"" << ins.back() << "\".";
                continue;
            }

            for( auto& opt : opts ) {
                if( strcmp( argv[ arg_idx ], opt.name ) != 0 )
                    continue;

                if( arg_idx + opt.arg == argc ) {
                    echo( this, ECHO_LEVEL_ERROR ) << "No arg: " << opt.name << ".";
                    return;
                }

                switch( opt.c ) {
                    case Opt::C_SRC_DIR: {
                        src_dir = argv[ arg_idx + 1 ];
                        echo( this, ECHO_LEVEL_OK ) << "Detected source dir: \"" << src_dir << "\".";
                    break; }

                    case Opt::C_OUT_DIR: {
                        out_dir = argv[ arg_idx + 1 ];
                        echo( this, ECHO_LEVEL_OK ) << "Detected output dir: \"" << out_dir << "\".";
                    break; }

                    case Opt::C_MODE: {
                        char* end_ptr = nullptr;
                        mode = strtol( argv[ arg_idx + 1 ], &end_ptr, 0xA );

                        echo( this, ECHO_LEVEL_OK ) << "Detected cvt mode: " << ( int )mode << ".";
                    break; }

                    case Opt::C_NORM: {
                        norm = true;
                        echo( this, ECHO_LEVEL_OK ) << "Detected normalization request.";
                    break; }
                }

                arg_idx += opt.arg;

                break;
            }
        }

        if( ins.empty() ) {
            echo( this, ECHO_LEVEL_ERROR ) << "No cvt candidates.";
            return;
        }

        if( IXT::UBYTE sit = ( src_dir.empty() << 1 ) | out_dir.empty(); sit != 0b00 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "No " << ( ( ( sit >> 1 ) & 1 ) ? "source dir." : "output dir." );
            return;
        }

        if( mode < 0 || mode > 2 ) {
            echo( this, ECHO_LEVEL_ERROR ) << "No/Ill-formed cvt mode: " << ( int )mode << ".";
            return;
        }

        *result = 0;
    }

    std::string                 src_dir   = {};
    std::string                 out_dir   = {};
    std::deque< std::string >   ins       = {};
    IXT::BYTE                   mode      = -1;
    bool                        norm      = false;
};


IXT::DWORD cvt_path( 
    const std::string& src_dir, 
    const std::string& out_dir, 
    IXT::BYTE          mode, 
    bool               norm,
    const std::string& rel 
) {
    File::for_each_in_dir_matching( src_dir.c_str(), rel.c_str(), [ & ] ( std::string_view mch ) -> IXT::DWORD {
        auto abs_path = src_dir + '/' + mch.data();
        
        comms( ECHO_LEVEL_PENDING ) << "Matched: \"" << abs_path.c_str() << "\".";

        std::ifstream in_file{ abs_path };

        if( !in_file ) {
            comms( ECHO_LEVEL_WARNING ) << "Fault opening for read. Proceeding.";
            goto l_end;
        }
    {
        std::vector< ggfloat_t > vrtxs = {};
        ggfloat_t                crd   = 0.0;
        Vec2                     max   = { std::numeric_limits< ggfloat_t >::min() };
        Vec2                     min   = { std::numeric_limits< ggfloat_t >::min() };

        vrtxs.reserve( 64 );

        while( in_file >> crd ) {
            vrtxs.emplace_back( crd ); 

            if( vrtxs.size() & 0x1 ) { if( crd > max.x ) max.x = crd; if( crd < min.x ) min.x = crd; }
            else                     { if( crd > max.y ) max.y = crd; if( crd < min.y ) min.y = crd; }
        }

        in_file.close();

        if( vrtxs.size() & 0x1 ) {
            comms( ECHO_LEVEL_WARNING ) << "Odd vertex count in source ( " << vrtxs.size() << " ). Check the file. Proceeding.";
            goto l_end;
        }

        IXT::BYTE header[ sizeof( XtFdx ) + sizeof( IXT::BYTE ) + sizeof( IXT::DWORD ) ];

        *( XtFdx* )header = FDX_CLUST2;
        *( ( IXT::BYTE* )header + sizeof( XtFdx ) ) = ( 0x0 & CLUST2_FILE_FMT_ORG_MSK ) | ( mode & CLUST2_FILE_FMT_MODE_MSK );
        *( IXT::DWORD* )( ( IXT::BYTE* )header + sizeof( XtFdx ) + sizeof( IXT::BYTE ) ) = vrtxs.size() >> 1;

        auto pos = mch.find_last_of( '.' );

        if( pos == decltype( mch )::npos ) 
            abs_path = out_dir + '/' + mch.data() + CLUST2_FILE_FMT_DFT_EXT.data();
        else
            abs_path = out_dir + '/' + std::string{ mch.data() }.substr( 0, pos - 1 ).data() + CLUST2_FILE_FMT_DFT_EXT.data();

        std::ofstream out_file{ abs_path.c_str() };

        if( !out_file ) {
            comms( ECHO_LEVEL_WARNING ) << "Fault opening for write: \"" << abs_path.c_str() << "\". Proceeding";
            goto l_end;
        }

        out_file.write( ( char* )header, sizeof( header ) );

        ggfloat_t w = norm ? ( max.x - min.x ) * 2.0_ggf : 1.0_ggf;
        ggfloat_t h = norm ? ( max.y - min.y ) * 2.0_ggf : 1.0_ggf;

        switch( mode ) {
            case 0b00: {
                auto itr = vrtxs.begin();

                while( itr != vrtxs.end() )
                    out_file << ( *itr++ / w ) << ' ' << ( *itr++ / h ) << '\n';
            break; }
        }

        out_file.close();

        comms() << "Match cvt done: " << abs_path.c_str() << ".\n";
    }
    l_end:
        return FILE_FEIDM_RESULT_ITR_CONTINUE;
    } );

    return 0;
}


int main( int argc, char* argv[] ) {
    if( argc < 2 ) {
        comms( ECHO_LEVEL_ERROR ) << "No input. Aborted.\n";
        return 1;
    }

    IXT::DWORD result = 0;

    CmdArgs cmd_args{ argc, argv, &result };
    
    if( result != 0 ) {
        comms( ECHO_LEVEL_ERROR ) << "Cmd line args ill-formed. Aborted.\n";
        return 1;
    }

    comms() << "Cmd line args parsed.\n";

    for( auto& in : cmd_args.ins ) {
        result = cvt_path( cmd_args.src_dir, cmd_args.out_dir, cmd_args.mode, cmd_args.norm, in ); 
    }

    comms() << "Done.\n";
}