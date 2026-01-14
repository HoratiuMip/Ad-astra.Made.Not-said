/**
 * @file: OSp/core.cpp
 * @brief: Implementation file.
 * @details: -
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>

namespace a113 {


A113_IMPL_FNC status_t _INTERNAL::init( int argc_, char* argv_[], const init_args_t& args_ ) {    
    A113_LOGI( "Hello there from AUTO-A113, version {}.{}.{}. Initializing the operating system plate...", A113_VERSION_MAJOR, A113_VERSION_MINOR, A113_VERSION_PATCH );

    int warn_count = 0x0;

    if( args_.flags & InitFlags_Sockets ) {
        A113_LOGD( "Initializing input/output sockets..." );

    #ifdef A113_TARGET_OS_WINDOWS
        int result = WSAStartup( 0x0202, &_Data.wsa_data );
        A113_ASSERT_OR( 0x0 == result ) {
            A113_LOGE( "Flawed WSA startup, [{}].", result );
            ++warn_count;
            goto l_bad_end;
        }

    #endif

        A113_LOGI( "Initialization of input/output sockets completed." ); 
        goto l_end;
    l_bad_end:
        A113_LOGW( "Flawed initialization of input/output sockets." );
    l_end:
    }

    if( 0x0 == warn_count ) A113_LOGI( "Initialization of the operating system plate completed flawlessly." );
    else A113_LOGW( "Initialization of the operating system plate completed with {} warnings.", warn_count );
    return 0x0;
}


};
