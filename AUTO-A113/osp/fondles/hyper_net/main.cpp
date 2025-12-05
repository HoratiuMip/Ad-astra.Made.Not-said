#include <a113/osp/hyper_net.hpp>

std::unique_ptr< a113::hyn::Executor > exec;
std::atomic_bool                       run;

int example_1( void ) {
    exec->push_port( new a113::hyn::Port{ { .str_id = "p0" } } );
    exec->push_port( new a113::hyn::Port{ { .str_id = "p1" } } );
    exec->push_port( new a113::hyn::Port{ { .str_id = "p2" } } );
    exec->push_route( new a113::hyn::Route{ { .str_id = "r0" } } );
    exec->push_route( new a113::hyn::Route{ { .str_id = "r1" } } );

    exec->bind_PRP( "p0", "r0", "p1", { .flight_mode = 1 }, {} );
    exec->bind_RP( "r0", "p2", {} );

    exec->bind_PRP( "p1", "r1", "p0", {}, {} );

    exec->inject( "p0", new a113::hyn::Token{ { .str_id = "t0" } } );

for(;run;) {
    exec->clock( 0.0 );
    std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
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
        a113::Log.critical( "Please call this program with the following arguments: <example_no>. Currently {} example(s) are available.", example_count );
        return -0x1;
    }

    try {
        int example_no = std::atoi( argv[ 1 ] );
        A113_ASSERT_OR( example_no >= 0x1 && example_no <= example_count ) throw std::runtime_error{ "Example number out of bounds." };

        exec = std::make_unique< a113::hyn::Executor >( std::format( "Example-{}", example_no ).c_str() );
        exec->set_log_level( spdlog::level::debug );

        run = true;
        return examples[ example_no - 1 ]();
    } catch( std::exception& ex ) {
        a113::Log.critical( "{}", ex.what() );
        return -0x1;
    }

    return 0x0;
}