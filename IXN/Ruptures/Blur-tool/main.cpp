/*
*/

#include <IXN/ring-0.hpp>
using namespace ixN;

#include <deque>


enum BLUR_BMP_RESULT {
    BLUR_BMP_RESULT_OK
};

dword_t blur_bmp_main_proc( Endec::Bmp& bmp, const auto& range ) {
    int      tmp_buf_sz = ( bmp.width * bmp.bytes_ps + bmp.padding ) * bmp.height;
    ubyte_t* tmp_buf    = ( ubyte_t* )malloc( tmp_buf_sz );

    memcpy( ( void* )tmp_buf, bmp.buffer + bmp.data_ofs, tmp_buf_sz );

    struct RCOff {
        int r; int c;
    };
    std::deque< RCOff > shfs;

    int w_sz = 15;

    for( int n = 0; n < w_sz; ++n )
        for( int m = 0; m < w_sz; ++m )
            shfs.emplace_back( RCOff{ r: w_sz / 2 - n, c: w_sz / 2 - m } );

    for( int row = 0; row < bmp.height; ++row ) {
        for( int col = 0; col < bmp.width; ++col ) {
            for( int ch = 0; ch < 3; ++ch ) {
                bool caged = false;

                for( auto& reg : range )
                    caged |= reg.contains( Vec2{ ( ggfloat_t )col, ( ggfloat_t )row } );
                
                if( !caged ) continue;

                uint32_t acc = 0.0;

                for( auto shf : shfs ) {
                    int fr = row + shf.r; if( fr < 0 || fr >= bmp.height ) continue;
                    int fc = col + shf.c; if( fc < 0 || fc >= bmp.width ) continue;

                    acc += bmp[ fr ][ bmp.bytes_ps * fc + ch ];
                } 

                ptrdiff_t ofs = &bmp[ row ][ bmp.bytes_ps * col + ch ] - ( ubyte_t* )bmp.buffer - bmp.data_ofs;

                *( tmp_buf + ofs ) = acc / shfs.size();
            }
        }
    }

    memcpy( bmp.buffer + bmp.data_ofs, ( void* )tmp_buf, tmp_buf_sz );

    free( tmp_buf );

    return BLUR_BMP_RESULT_OK;
}


int main( int argc, char* argv[] ) {
    Surface surf{ "Blur-tool", { .0, .0 }, { std::min( Env::w<2.>(), Env::h<2.>() ) }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID };

    Endec::Bmp           bmp  = {};
    std::deque< Clust2 > regz = {};
    std::mutex           sync = {};
    std::atomic< bool >  sreq = { false };

    surf.on< SURFACE_EVENT_FILEDROP >( [ & ] ( std::vector< std::string > files, [[maybe_unused]]auto& ) -> void {
        sreq.store( true, std::memory_order_relaxed );
        std::unique_lock lock{ sync };

        for( auto& file : files ) {
            if( file.ends_with( ".clst2" ) ) {
                regz.emplace_back( file.c_str() );
            } else if( file.ends_with( ".bmp" ) ){
                bmp.~Bmp();
                new ( &bmp ) Endec::Bmp{ file };
            }
        }

        sreq.store( false, std::memory_order_relaxed );
        sreq.notify_one();
    } );

    surf.on< SURFACE_EVENT_KEY >( [ & ] ( SurfKey key, SURFKEY_STATE state, [[maybe_unused]]auto& ) -> void {
        if( state != SURFKEY_STATE_UP ) goto l_key_down;

        switch( key ) {
            case SurfKey::ENTER: {
                if( surf.ptr_v().mag() > 0.05 ) break;
                if( bmp.buffer == nullptr ) break;
                if( regz.empty() ) break;

                Ticker tick{};
                dword_t result = blur_bmp_main_proc( bmp, regz );
                double elapsed = tick.lap();

                Echo{}( nullptr, EchoLevel_Info ) << "Blurring took: " << elapsed << "s.";

                switch( result ) {
                    case BLUR_BMP_RESULT_OK: {
                        std::string path = OS::file_browse_save( "Save processed bmp:" );

                        if( path.empty() ) break;

                        bmp.write_file( path );

                    break; }
                }
            break; }
        }   

l_key_down:
        return;
    } );

    auto render_th = std::thread{ [ & ] () -> void {
        Renderer2 render{ surf };

        RdlSweep2 inv_sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .12, .12 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 230, 10, 10 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, 1.0 }
            }, surf.height() 
        };

        RdlSweep2 bmp_sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .12, .12 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 90, 230, 10 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, 1.0 }
            }, surf.height() 
        };

        RdlSweep2 reg_sweep = { 
            render, { .0, .0 }, { .0, .0 }, { .12, .12 }, {
                Sweep2gcn_t{ { 10, 10, 20 }, .0 },
                Sweep2gcn_t{ { 90, 10, 230 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, .62 },
                Sweep2gcn_t{ { 0, 0, 0, 0 }, 1.0 }
            }, surf.height() 
        };

        while( !surf.down( SurfKey::ESC ) ) {
            render.charge().fill( RGBA{ 10, 10, 20 } );

            if( bmp.buffer == nullptr ) {
                inv_sweep.org_at( surf.ptr_v() );
                render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, inv_sweep );
                goto l_end;
            }
            {
            Vec2 ref = surf.ptr_v();

            sreq.wait( true, std::memory_order_relaxed );
            std::unique_lock lock{ sync };

            ggfloat_t theta_step = 360.0 / ( regz.size() + 1 );
            for( int n = 1; n <= regz.size(); ++n ) {
                reg_sweep.org_at( ref.spinned( theta_step * n ) );
                render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, reg_sweep );
            }

            bmp_sweep.org_at( ref );
            render.line( Vec2{ -.5, .0 }, Vec2{ .5, 0 }, bmp_sweep );
            }
l_end:
            render.splash();
        }
    } };

    while( !surf.down( SurfKey::ESC ) ) {
        std::this_thread::sleep_for( std::chrono::milliseconds{ 1000 } );
    }

    render_th.join();
}