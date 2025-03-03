#include <IXT/init.hpp>
#include <IXT/comms.hpp>
#include <IXT/hyper-vector.hpp>
#include <IXT/render2.hpp>
#include <IXT/SpecMod/barracuda-ctrl-driver.hpp>

#include <conio.h>


using namespace barracuda_ctrl;
using namespace IXT;
using namespace IXT::SpecMod;


struct _VISUAL {
    HVEC< Surface >     surf;
    HVEC< Renderer2 >   render;

    void init( void ) {
        ggfloat_t surf_size = std::min( Env::width(), Env::height() ) * 0.8;
        surf.vector( HVEC< Surface >::allocc( "BarraCUDA-CTRL Tester", Crd2{ 10 }, Vec2{ surf_size }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID ) );

        render.vector( HVEC< Renderer2 >::allocc( *surf ) );
    }  
} VISUAL;


BarracudaController   CTRL;
dynamic_state_t       DYNAMIC;


struct BOARD {
    BOARD( int gi, int gj ) 
    : view{ VISUAL.render, Crd2{ 0.25f * gi, 0.25f * gj }, Vec2{ 0.25 } }
    {}

    virtual ~BOARD() {}

    Viewport2   view;

    virtual void frame( void ) = 0;

    void _begin( void ) { 
        view.rs2_uplink(); 
        //view.restrict(); 
        view.splash_bounds( RENDERER2_DFT_SWEEP_WHITE ); 
    } 
    void _end( void ) { 
        //view.lift_restrict(); 
        view.rs2_downlink(); 
    };
};

struct JOYSTICK_BOARD : BOARD {
    JOYSTICK_BOARD( int gi, int gj, joystick_t* ptr )
    : BOARD{ gi, gj }, dat{ ptr }
    {}

    joystick_t*   dat;

    virtual void frame( void ) override {
        this->_begin();

        if( dat->sw.dwn ) {
            view.line( Vec2{ -0.5, 0.5 }, Vec2{ 0.5, -0.5 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_WHITE ) );
            view.line( Vec2{ -0.5, -0.5 }, Vec2{ 0.5, 0.5 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_WHITE ) );
        }

        view.line( Vec2{ 0, 0 }, Vec2{ dat->x / 2, dat->y / 2  }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_MAGENTA ) );
        view.line( Vec2{ 0, 0 }, Vec2{ dat->x / 2, 0 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_GREEN ) );
        view.line( Vec2{ 0, 0 }, Vec2{ 0, dat->y / 2 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_BLUE ) );

        this->_end();
    }
};

struct MAIN_SWITCHES_BOARD : BOARD {
    MAIN_SWITCHES_BOARD( int gi, int gj )
    : BOARD{ gi, gj }
    {}

    virtual void frame( void ) override {
        this->_begin();

        struct _BATCH {
            switch_t&   sw;
            ggfloat_t   x;
            Sweep2&     sweep;
        } batches[] = {
            { sw: DYNAMIC.giselle,  x: -0.3f, sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_BLUE ) },
            { sw: DYNAMIC.karina ,  x: -0.1f, sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_RED ) },
            { sw: DYNAMIC.ningning, x: 0.1f,  sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_YELLOW ) },
            { sw: DYNAMIC.winter,   x: 0.3f,  sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_GREEN ) }
        };

        for( auto& batch : batches ) {
            ggfloat_t y = batch.sw.dwn ? 0.5 : 0.1;
            view.line( Vec2{ batch.x, -y }, Vec2{ batch.x, y }, batch.sweep );
        }

        this->_end();
    }
};


enum {
    READY, RESET, EXIT
} STATUS;


struct _DASHBOARD {
    std::thread                             th;
    std::atomic_bool                        ready;

    std::list< std::unique_ptr< BOARD > >   boards;

    void begin( void ) {
        ready.store( true, std::memory_order_relaxed );
        th = std::thread{ [ & ] () -> void { this->loop(); } };
    }

    void loop( void ) {
        while( STATUS == READY ) {
            VISUAL.render->rs2_uplink().fill( RGBA{ 0.1, 0.1, 0.1 } );

            for( auto& board : boards ) board->frame();

            VISUAL.render->rs2_downlink();

            if( CTRL.listen_trust( &DYNAMIC ) != 0 ) STATUS = RESET;
        }
    }
} DASHBOARD;


struct _COMMAND {
    bool exec( std::string& cmd ) {
        const char* cmds[] = {
            "none", "exit", "ping"
        };

        ptrdiff_t idx = std::find_if( cmds, cmds + std::size( cmds ), [ &cmd ] ( const char* entry ) -> bool {
            return cmd == entry;
        } ) - cmds;

        switch( idx ) {
            case 0: return true;

            case 1: { STATUS = EXIT; break; }

            case 2: { CTRL.ping(); break; }

            default: return false;
        }

        return true;
    }

} COMMAND;


int main( int argc, char* argv[] ) {
    initial_uplink( argc, argv, INIT_FLAG_UPLINK_NETWORK, nullptr, nullptr );

l_attempt_connect:
    comms( ECHO_LEVEL_PENDING ) << "Waiting for connection...";
    if( CTRL.data_link( BARRACUDA_CTRL_FLAG_TRUST ) != 0 ) {
        comms() << "Could not connect to the controller. Retrying in 3s...\n";
        std::this_thread::sleep_for( std::chrono::seconds{ 3 } );
        goto l_attempt_connect;
    }
    comms() << "Connected to the controller.\n";

    VISUAL.init();

    DASHBOARD.boards.emplace_back( new JOYSTICK_BOARD{ 1, 2, &DYNAMIC.rachel } );
    DASHBOARD.boards.emplace_back( new JOYSTICK_BOARD{ 3, 1, &DYNAMIC.samantha } );
    DASHBOARD.boards.emplace_back( new MAIN_SWITCHES_BOARD{ 3, 2 } );

    DASHBOARD.begin();
    
    STATUS = READY;
    do {
        comms( ECHO_LEVEL_INPUT ) << "Command: ";

        while( !kbhit() ) { 
            std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } );
            if( STATUS != READY ) goto l_main_loop_break;
        }

        std::unique_lock lock{ comms.mtx() };
        std::string cmd; std::cin >> cmd;
        lock.unlock();

        if( STATUS != READY ) goto l_main_loop_break;

        if( COMMAND.exec( cmd ) ) 
            comms( ECHO_LEVEL_OK ) << "Command \"" << cmd << "\" executed.\n";
        else
            comms( ECHO_LEVEL_ERROR ) << "Invalid command \"" << cmd << "\".\n";

    } while( STATUS == READY );
l_main_loop_break:

    //if( STATUS == RESET ) goto l_attempt_connect;

    DASHBOARD.ready.store( false, std::memory_order_relaxed );
    DASHBOARD.th.join();

    final_downlink( argc, argv, 0, nullptr, nullptr );
}