#include <a113/osp/hyper_net.hpp>
#include <fstream>
#include <conio.h>

std::unique_ptr< a113::hyn::Executor > exec;
std::atomic_bool                       run;

#define GRAPHVIZ_PATH "hyn_graphviz.txt"
struct _flags_t {
    bool   make_graphviz   = false;
} flags;

void make_graphviz( void ) {
    if( not flags.make_graphviz ) return;
    
    std::ofstream{ GRAPHVIZ_PATH } << exec->Utils_make_graphviz();
}

int example_1( void ) {
    exec->push_port( new a113::hyn::Port{ { .str_id = "p0" } } );
    exec->push_port( new a113::hyn::Port{ { .str_id = "p1" } } );
    exec->push_port( new a113::hyn::Port{ { .str_id = "p2" } } );
    exec->push_port( new a113::hyn::Port{ { .str_id = "p3" } } );
    exec->push_route( new a113::hyn::Route{ { .str_id = "r0" } } );
    exec->push_route( new a113::hyn::Route{ { .str_id = "r1" } } );
    exec->push_route( new a113::hyn::Route{ { .str_id = "r2" } } );
    exec->push_route( new a113::hyn::Route{ { .str_id = "r3" } } );

    exec->bind_PRP( "p0", "r0", "p1", { .flight_mode = 2 }, {} );
    exec->bind_RP( "r0", "p2", {} );
    exec->bind_RP( "r0", "p3", {} );

    exec->bind_PR( "p2", "r2", { .min_tok_cnt = 5 } );
    exec->bind_PR( "p3", "r3", { .min_tok_cnt = 5, .rte_tok_cnt = 3 } );

    exec->bind_PRP( "p1", "r1", "p0", {}, {} );

    exec->inject( "p0", new a113::hyn::Token{ { .str_id = "t0" } } );

    make_graphviz();

for(;run;) {
    if( 'X' == _getch() ) break;
    exec->clock( 0.0 );
    make_graphviz();
}
    return 0x0;
}

int ( *examples[] )( void ) = {
    &example_1
};
constexpr int example_count = sizeof( examples ) / sizeof( void* );

int main( int argc, char* argv[] ) {
    a113::init( argc, argv, a113::init_args_t{
        .flags = a113::InitFlags_None
    } );

    A113_ASSERT_OR( argc > 1 ) {
        A113_LOGE( "Please call this program with the following arguments: <example_no> [--make-graphviz]. Currently {} example(s) are available.", example_count );
        return -0x1;
    }

    for( auto idx{ 0x1uz }; idx < argc; ++idx ) {
        if( NULL == strcmp( "--make-graphviz", argv[ idx ] ) ) flags.make_graphviz = true;

    }

    try {
        int example_no = std::atoi( argv[ 1 ] );
        A113_ASSERT_OR( example_no >= 0x1 && example_no <= example_count ) throw std::runtime_error{ "Example number out of bounds." };

        exec = std::make_unique< a113::hyn::Executor >( std::format( "Example-{}", example_no ).c_str() );
        //exec->set_log_level( spdlog::level::debug );

        run = true;
        return examples[ example_no - 1 ]();
    } catch( std::exception& ex ) {
        A113_LOGC( "{}", ex.what() );
        return -0x1;
    }

    return 0x0;
}