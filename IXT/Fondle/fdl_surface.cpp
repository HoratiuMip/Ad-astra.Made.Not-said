/*
*/
#include <IXT/surface.hpp>
#include <IXT/os.hpp>

#include <list>
#include <vector>
#include <string>
#include <mutex>
#include <memory>

using namespace IXT;



struct MyKeySeqTrigger : public Descriptor {
    IXT_DESCRIPTOR_STRUCT_NAME_OVERRIDE( "MyKeySeqTrigger" );

    XtDx xtdx() const override {
        return ( void* )this->struct_name().data();
    };


    bool   triggered   = false;


    void init( Surface& surf, IXT_COMMS_ECHO_ARG ) {
        surf.socket_plug< SURFACE_EVENT_KEY >( 
            this->xtdx(), SURFACE_SOCKET_PLUG_AT_EXIT,
            [ this, &surf, seq_at = 0ULL ] ( SurfKey key, SURFKEY_STATE state, SurfaceTrace& trace ) mutable -> void {
                if( state == SURFKEY_STATE_UP ) return;

                if( triggered )
                    std::cout << '\a';

                if( key == seq_at++[ "SWITCH" ] ) {
                    if( seq_at != strlen( "SWITCH" ) ) return;
                    
                    ( triggered ^= true );// ? surf.solidify() : surf.liquify();
                }

                seq_at = 0;
            }
        );

        echo( this, ECHO_STATUS_INTEL ) << "MyKeySeqTrigger engaged.";
    }

    void kill( Surface& surf ) {
        surf.socket_unplug( this->xtdx() );
    }
};



int main() {
    Surface surface{ "IXT Surface", Crd2{ 64 }, Vec2{ 512 }, SURFACE_STYLE_LIQUID};
    surface.uplink( SURFACE_THREAD_ACROSS );
    surface.downlink();


    std::list< SurfKey > pressed{};

    std::vector< std::string > last_files{};
    std::mutex last_files_mtx;
    size_t last_files_str_size = 0;
    int hscroll = 0;
    int vscroll = 0;


    surface.on< SURFACE_EVENT_KEY >( [ &pressed, &surface ] ( SurfKey key, SURFKEY_STATE state, SurfaceTrace& trace ) -> void {
        if( key != SurfKey::ENTER && std::ranges::find( pressed, key ) == pressed.end() ) {
            pressed.push_back( key );
        }

        if( state != SURFKEY_STATE_DOWN ) return;

        switch( key ) {
            case SurfKey::LMB: {
                static Crd2 poss[] = {
                    { 50 }, { 300 }, { 50, 300 }, { 300, 50 }
                };
                static int8_t poss_at = 0;

                surface.relocate( poss[ poss_at ] );
                ( ++poss_at ) %= std::size( poss );
            break; }

            case SurfKey::RMB: {
                static Vec2 szs[] = {
                    { 64 }, { 128 }, { 256 }, { 512 }
                };
                static int8_t szs_at = 0;

                surface.resize( szs[ szs_at ] );
                ( ++szs_at ) %= std::size( szs );
            break; }

            case SurfKey::MMB: {
                static bool hidden = false;

                ( hidden ^= true ) ? surface.hide_def_ptr() : surface.show_def_ptr();
            break; }
        }
    } );

    surface.on< SURFACE_EVENT_FILEDROP >( [ &last_files, &last_files_mtx ] ( auto files, SurfaceTrace& trace ) -> void {
        std::unique_lock lock{ last_files_mtx };
        last_files = std::move( files );
    } );

    surface.on< SURFACE_EVENT_SCROLL >( [ &vscroll, &hscroll ] ( Vec2, SURFSCROLL_DIRECTION direction, SurfaceTrace& trace ) -> void {
        vscroll += 1 * ( direction == SURFSCROLL_DIRECTION_UP ) - 1 * ( direction == SURFSCROLL_DIRECTION_DOWN );
        hscroll += 1 * ( direction == SURFSCROLL_DIRECTION_RIGHT ) - 1 * ( direction == SURFSCROLL_DIRECTION_LEFT );
    } );


    std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
    surface.uplink( SURFACE_THREAD_ACROSS );


    MyKeySeqTrigger my_seq_trigger;
    my_seq_trigger.init( surface );


    auto initial_crs = OS::console.crs();

    std::cout << std::boolalpha;

    while( !surface.down( SurfKey::ESC ) ) {
        OS::console.crs_at( initial_crs );
        

        std::cout << std::fixed
                  << "Pointer: "
                  << std::setprecision( 0 ) 
                  << "\n\tVL: x[" << surface.ptr_vl().x << "] y[" << surface.ptr_vl().y << "] "
                  << std::setprecision( 3 ) 
                  << "\n\tVG: x[" << surface.ptr_vg().x << "] y[" << surface.ptr_vg().y << "] "
                  << std::setprecision( 0 ) 
                  << "\n\tCL: x[" << surface.ptr_cl().x << "] y[" << surface.ptr_cl().y << "] "
                  << std::setprecision( 3 ) 
                  << "\n\tCG: x[" << surface.ptr_cg().x << "] y[" << surface.ptr_cg().y
                  << std::setw( 10 ) << std::left << "]." << "\n\n";

        std::cout << "Position: x[" << surface.pos().x << "] y[" << surface.pos().y
                  << std::setw( 10 ) << std::left << "]." << '\n';

        std::cout << "Size:     w[" << surface.width() << "] h[" << surface.height()
                  << std::setw( 10 ) << std::left << "]." << "\n\n";

        
        std::cout << "Keys: ";
        for( auto key : pressed ) {
            OS::console.clr_with( surface.down( key ) ? OS::CONSOLE_CLR_GREEN : OS::CONSOLE_CLR_BLUE );

            std::cout << ( char )key << "[" << key << "] ";
        }

        OS::console.clr_with( OS::CONSOLE_CLR_WHITE );
        std::cout << '\n';

        std::cout << "MyKeySeqTrigger triggered: " << std::setw( 5 ) << std::left << my_seq_trigger.triggered << "\n\n";


        {
            std::cout << "Last dropped files: ";

            std::unique_lock lock{ last_files_mtx };

            size_t size = 0;

            for( auto& file : last_files ) {
                std::cout << "[" << file << "] ";
                size += file.size() + 3;
            }

            if( size < last_files_str_size ) 
                std::cout << std::setw( last_files_str_size - size + 1 ) << std::right;
            last_files_str_size = size;

            std::cout << "\n\n";
        }

        std::cout << "V-Scroll: [" << vscroll << std::setw( 10 ) << std::left << "]." << '\n';
        std::cout << "H-Scroll: [" << hscroll << std::setw( 10 ) << std::left << "]." << "\n\n";


        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
}