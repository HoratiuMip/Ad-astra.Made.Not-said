/**
 * @file: OSp/core.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/OSp/core.hpp>

namespace A113 { namespace OSp {


A113_OS_FNC RESULT INTERNAL::init( int argc_, char* argv_[], const init_args_t& args_ ) {
    if( nullptr == ( Log.Default = spdlog::stdout_color_mt( A113_VERSION_STRING ) ) ) return -0x1;
    Log.make_default( Log.Default );
    Log->info( "Hello there from AUTO-A113, version {}.{}.{}. Initializing the operating system plate...", A113_VERSION_MAJOR, A113_VERSION_MINOR, A113_VERSION_PATCH );

    int warn_count = 0x0;

    if( nullptr == ( Log.IO = spdlog::stdout_color_mt( A113_VERSION_STRING"//IO" ) ) ) {
        Log->warn( "Flawed creation of the IO logger, moving it to the default one." );
        ++warn_count;
        Log.IO = Log.Default;
    } else {
        Log.make_default( Log.IO );
    }

    if( args_.flags & InitFlags_Sockets ) {
        Log.IO->debug( "Initializing input/output sockets..." );

    #ifdef A113_TARGET_OS_WINDOWS
        int result = WSAStartup( 0x0202, &_Data.wsa_data );
        A113_ASSERT_OR( 0x0 == result ) {
            Log.IO->error( "Flawed WSA startup, [{}].", result );
            ++warn_count;
            goto l_bad_end;
        }

    #endif

        Log.IO->info( "Initialization of input/output sockets completed." ); 
        goto l_end;
    l_bad_end:
        Log.IO->warn( "Flawed initialization of input/output sockets." );
    l_end:
    }

    if( 0x0 == warn_count ) Log->info( "Initialization of the operating system plate completed flawlessly." );
    else Log->warn( "Initialization of the operating system plate completed with {} warnings.", warn_count );
    return 0x0;
}


} };
