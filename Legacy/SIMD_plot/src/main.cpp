/*



*/


#define IXT_ECHO
#define IXT_OS_WINDOWS
#define IXT_GL_DIRECT
#include "C:\\Hackerman\\Git\\For_Academical_Purposes\\Back\\1712301362__9T.cpp"
using namespace IXT;

#include <immintrin.h>



void add_8_ints_n_times_simd( __m256i& acc, size_t n_req ) {
    __m256i val = _mm256_set1_epi32( 1 );

    for( size_t n = 1; n <= n_req; ++n )
        acc = _mm256_add_epi32( acc, val );
}

void add_8_ints_n_times_serial( int* arr, size_t n_req ) {
    for( size_t n = 1; n <= n_req; ++n ) 
        for( size_t idx = 0; idx < 8; ++idx )
            ++arr[ idx ];
}



int main( int argc, char* argv[] ) {
    Surface surf{ "SIMD Performance", { 0, 0 }, { 800, 800 }, SURFACE_THREAD_ACROSS };

    Renderer2 render{ surf };

    Viewport2 viewport{ render, Coord<>{ 0, 0 }, { surf.width(), surf.height() } };
    viewport.uplink();

    System2 system{
        viewport, 
        System2Packet {
            div: {
                px:       { 80, 80 },
                px_max:   { 300, 300 },
                px_min:   { 40, 40 },
                hstk:     10,
                mean:     { 1.0, 0.3 }
            },
            brushes: {
                nodes: { 
                    std::make_shared< SolidBrush2 >( render, Chroma{ 17, 249, 226, 255 }, 4.0 ),
                    std::make_shared< SolidBrush2 >( render, Chroma{ 230, 0, 255, 255 }, 4.0 ),
                    std::make_shared< SolidBrush2 >( render, Chroma{ 0, 255, 9, 255 }, 4.0 ),
                    std::make_shared< SolidBrush2 >( render, Chroma{ 180, 52, 255, 255 }, 4.0 ),
                    std::make_shared< SolidBrush2 >( render, Chroma{ 0, 255, 205, 255 }, 4.0 ),
                    std::make_shared< SolidBrush2 >( render, Chroma{ 182, 255, 12, 255 }, 4.0 ),
                    std::make_shared< SolidBrush2 >( render, Chroma{ 0, 26, 255, 255 }, 4.0 )
                },
                bgnd:  std::make_shared< SolidBrush2 >( render, Chroma{ 22, 23, 25, 255 } ),
                axis:  std::make_shared< SolidBrush2 >( render, Chroma{ 200, 200, 200, 255 }, 2.0 )
            }
        }
    };
    system.uplink_auto_roam();


    Clock clk{};
    Clock rclk{};
    

    __m256i v_simd = _mm256_set1_epi32( 0 );
    int v_serial[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    enum {
        SIMD = 0, Serial = 1
    };

    double acc[] = { 0.0, 0.0 };
    size_t step = 0;
    size_t at_steps = 1;
    size_t at_steps_mod = 1;
    double refresh_threshold = 1.0;


    for( size_t n = 1; n <= 4; ++n ) {
        static std::string line_name = "line0";

        Shared< SolidBrush2 > line_brush{ new SolidBrush2{ render, Chroma{ 100, 100, 100, 255 }, 1.0 } };

        *( line_name.end() - 1 ) = static_cast< char >( n + '0' );
        system.insert( { line_name, System2Node::Function{ [ n ] ( double x ) -> double {
            return n;
        } } } ).first->second.give_brush( line_brush );
    }


    std::string graph_data_base_name = "data0";
    auto* graph_data = &system.insert( { graph_data_base_name, System2Node::Collection{} } ).first->second.collection();
    graph_data->emplace_back( 0, 0 );


    surf.on< SURFACE_EVENT_KEY >( [ & ]( Key key, KEY_STATE state, [[ maybe_unused ]] auto& ) -> void {
        if( state != KEY_STATE_DOWN ) return;

        switch( key ) {
            case Key::N: {
                ++graph_data_base_name[ 4 ];

                auto& ref = system.insert( { graph_data_base_name, System2Node::Collection{} } ).first->second.collection();
                ref.emplace_back( 0, 0 );

                acc[ SIMD ] = acc[ Serial ] = 0.0;
                step = 0;
                at_steps = 1;
                at_steps_mod = 1;

                graph_data = &ref;
            break; }

            case Key::UP: refresh_threshold = std::clamp( refresh_threshold + 0.1, 0.1, 3.0 ); break;
            case Key::DOWN: refresh_threshold = std::clamp( refresh_threshold - 0.1, 0.1, 3.0 ); break;
        }
    } );


    std::thread measure_thread{ [ & ] () -> void { 
        CONSOLE_SCREEN_BUFFER_INFO sb_info{};
        GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &sb_info );


        while( true ) {
            ++step;

            clk.lap();
            add_8_ints_n_times_simd( v_simd, at_steps );
            double elapsed_simd = clk.lap();
            acc[ SIMD ] += elapsed_simd;

            clk.lap();
            add_8_ints_n_times_serial( v_serial, at_steps );
            double elapsed_serial = clk.lap();
            acc[ Serial ] += elapsed_serial;


            if( rclk.peek_lap() >= refresh_threshold ) {
                SetConsoleCursorPosition( 
                    GetStdHandle( STD_OUTPUT_HANDLE ), 
                    COORD{ sb_info.dwCursorPosition.X, sb_info.dwCursorPosition.Y } 
                );

                auto simd_mean = acc[ SIMD ] / step;
                auto serial_mean = acc[ Serial ] / step;
                auto factor = acc[ Serial ] / acc[ SIMD ];


                graph_data->emplace_back( graph_data->back().x + 1, factor );


                std::cout << "SIMD:   " << simd_mean << "       \n";
                std::cout << "Serial: " << serial_mean << "       \n";
                std::cout << "SIMD faster by: " << factor << "       \n";

                std::cout << "SIMD Acc:   ";
                for( size_t idx = 0; idx < 1; ++idx )
                    std::cout << ( ( int* ) &v_simd )[ idx ] << ' ';
                std::cout << "       \n";

                std::cout << "Serial Acc: ";
                //for( auto& i : v_serial )
                    std::cout << v_serial[ 0 ] << ' ';
                std::cout << "       \n";

                std::cout << "Increments: " << at_steps << "       \n";
                std::cout << "Refresh threshold: " << refresh_threshold << "       \n";
                

                acc[ SIMD ] = acc[ Serial ] = 0.0;
                step = 0;

                rclk.lap();


                if( at_steps == at_steps_mod * 10 )
                    at_steps_mod *= 10;

                at_steps += at_steps_mod;
            }
        }
    } };


    while( true ) {
        render.open();
        render( system );
        render.execute();
    }


    measure_thread.join();

    return 0;
}
