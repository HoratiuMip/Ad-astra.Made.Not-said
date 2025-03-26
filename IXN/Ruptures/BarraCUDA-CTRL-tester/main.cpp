#include <IXN/init.hpp>
#include <IXN/comms.hpp>
#include <IXN/hyper-vector.hpp>
#include <IXN/render2.hpp>
#include <IXN/Device/barracuda-ctrl-nln-driver.hpp>

#include <conio.h>


using namespace barcud_ctrl;
using namespace ixN;
using namespace ixN::Dev;


struct _VISUAL {
    HVEC< Surface >     surf;
    HVEC< Renderer2 >   render;

    void init( void ) {
        ggfloat_t surf_size = std::min( Env::width(), Env::height() ) * 0.8;
        surf.vector( HVEC< Surface >::allocc( "BarraCUDA-CTRL Tester", Crd2{ 10 }, Vec2{ surf_size * 1.25f, surf_size }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID ) );

        render.vector( HVEC< Renderer2 >::allocc( *surf ) );
    }  
} VISUAL;


BarracudaCTRL CTRL;


struct BOARD {
    BOARD( int gi, int gj ) 
    : view{ VISUAL.render, Crd2{ 0.25f * 0.8f * gi, 0.25f * gj }, Vec2{ 0.25f * 0.8f, 0.25f } }
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
            view.line( Vec2{ 0.0, -0.5 }, Vec2{ 0.0, 0.5 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_WHITE ) );
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
            { sw: CTRL.dynamic.giselle,  x: -0.3f, sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_BLUE ) },
            { sw: CTRL.dynamic.karina ,  x: -0.1f, sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_RED ) },
            { sw: CTRL.dynamic.ningning, x: 0.1f,  sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_YELLOW ) },
            { sw: CTRL.dynamic.winter,   x: 0.3f,  sweep: VISUAL.render->pull( RENDERER2_DFT_SWEEP_GREEN ) }
        };

        for( auto& batch : batches ) {
            ggfloat_t y = batch.sw.dwn ? 0.5 : 0.1;
            view.line( Vec2{ batch.x, -y }, Vec2{ batch.x, y }, batch.sweep );
        }

        this->_end();
    }
};

struct GYRO_TRANSLATION_ACC_BOARD : BOARD {
    GYRO_TRANSLATION_ACC_BOARD( int gi, int gj, gyro_t* ptr )
    : BOARD{ gi, gj }, dat{ ptr }
    {}

    gyro_t*   dat;

    virtual void frame( void ) override {
        this->_begin();

        float mul = 1.0 / 4.0;

        view.line( Vec2{ 0.0, 0.0 }, Vec2{ dat->acc.x * mul, dat->acc.y * mul }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_MAGENTA ) );
        view.line( Vec2{ 0.0, 0.0 }, Vec2{ dat->acc.x * mul, 0.0 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_RED ) );
        view.line( Vec2{ 0.0, 0.0 }, Vec2{ 0.0, dat->acc.y * mul }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_BLUE ) );
        view.line( Vec2{ -0.3, 0.0 }, Vec2{ -0.3, dat->acc.z * mul }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_GREEN ) );

        this->_end();
    }
};

struct GYRO_ROTATION_ACC_BOARD : BOARD {
    GYRO_ROTATION_ACC_BOARD( int gi, int gj, gyro_t* ptr )
    : BOARD{ gi, gj }, dat{ ptr }
    {}

    gyro_t*   dat;

    virtual void frame( void ) override {
        this->_begin();

        float mul = 1.0 / 500.0;

        view.line( Vec2{ 0.0, 0.0 }, Vec2{ dat->gyr.x * mul, dat->gyr.y * mul }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_MAGENTA ) );
        view.line( Vec2{ 0.0, 0.0 }, Vec2{ dat->gyr.x * mul, 0.0 }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_RED ) );
        view.line( Vec2{ 0.0, 0.0 }, Vec2{ 0.0, dat->gyr.y * mul }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_BLUE ) );
        view.line( Vec2{ -0.3, 0.0 }, Vec2{ -0.3, dat->gyr.z * mul }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_GREEN ) );

        this->_end();
    }
};

struct LIGHT_SENSOR_BOARD : BOARD {
    LIGHT_SENSOR_BOARD( int gi, int gj, float* ptr )
    : BOARD{ gi, gj }, dat{ ptr }
    {}

    float*   dat;

    virtual void frame( void ) override {
        this->_begin();

    
        view.line( Vec2{ -0.25, -0.5f }, Vec2{ -0.25, *dat - 0.5f }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_WHITE ) );
        view.line( Vec2{ 0, -0.5f }, Vec2{ 0.0, *dat - 0.5f }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_WHITE ) );
        view.line( Vec2{ 0.25, -0.5f }, Vec2{ 0.25, *dat - 0.5f }, VISUAL.render->pull( RENDERER2_DFT_SWEEP_WHITE ) );

        this->_end();
    }
};



enum {
    READY, RESET, EXIT
} STATUS;


struct _DASHBOARD {
    std::thread                             th;
    std::list< std::unique_ptr< BOARD > >   boards;

    void begin( void ) {
        th = std::thread{ [ & ] () -> void { this->loop(); } };
    }

    void loop( void ) {
        while( STATUS == READY ) {
            VISUAL.render->rs2_uplink().fill( RGBA{ 0.1, 0.1, 0.1 } );

            for( auto& board : boards ) board->frame();

            VISUAL.render->rs2_downlink();

            BAR_PROTO_STREAM_RESOLVE_RECV_INFO info;
            if( CTRL.trust_resolve_recv( &info ) <= 0 ) STATUS = RESET;
        }
    }
} DASHBOARD;


struct _COMMAND {
    bool exec( std::string& cmd ) {
        const char* cmds[] = {
            "none", "exit", "ping", "get-byte", "set-byte", "proto-breach"
        };

        ptrdiff_t idx = std::find_if( cmds, cmds + std::size( cmds ), [ &cmd ] ( const char* entry ) -> bool {
            return cmd == entry;
        } ) - cmds;

        switch( idx ) {
            case 0: return true;

            case 1: { STATUS = EXIT; break; }

            case 2: { CTRL.ping(); break; }

            case 3: {
                std::string str_id; std::cin >> str_id;
                char data; 
                if( CTRL.get( str_id, &data, 1 ) == 0 ) {
                    comms( EchoLevel_Ok ) << "\"" << str_id << "\" = " << std::hex << ( int )data << std::dec << ".";
                } else {
                    comms( EchoLevel_Warning ) << "\"" << str_id << "\" NAK'd.";
                }
            break; }

            case 4: {
                std::string str_id; std::cin >> str_id;
                char data; int n; std::cin >> n; data = ( char )n; comms( EchoLevel_Info ) << "Setting " << str_id << " to " << n << ".";
                CTRL.set( str_id, &data, 1 );
            break; }

            case 5: {
                int x = 1;
                CTRL.trust_burst( &x, 4, 0 );
            break; }

            default: return false;
        }

        return true;
    }

} COMMAND;


int main( int argc, char* argv[] ) {
    if( ixN::begin_runtime( argc, argv, BEGIN_RUNTIME_FLAG_INIT_NETWORK, nullptr, nullptr ) != 0 ) goto l_main_loop_break;

    VISUAL.init();

    DASHBOARD.boards.emplace_back( new JOYSTICK_BOARD{ 1, 2, &CTRL.dynamic.rachel } );
    DASHBOARD.boards.emplace_back( new JOYSTICK_BOARD{ 3, 1, &CTRL.dynamic.samantha } );
    DASHBOARD.boards.emplace_back( new MAIN_SWITCHES_BOARD{ 3, 2 } );
    DASHBOARD.boards.emplace_back( new GYRO_TRANSLATION_ACC_BOARD{ 1, 1, &CTRL.dynamic.gran } );
    DASHBOARD.boards.emplace_back( new GYRO_ROTATION_ACC_BOARD{ 2, 1, &CTRL.dynamic.gran } );
    DASHBOARD.boards.emplace_back( new LIGHT_SENSOR_BOARD{ 1, 0, &CTRL.dynamic.naksu } );

l_attempt_connect:
    comms( EchoLevel_Pending ) << "Waiting for connection...";
    if( CTRL.connect( BARRACUDA_CTRL_FLAG_TRUST_INVOKER ) != 0 ) {
        comms() << "Could not connect to the controller. Retrying in 3s...\n";
        std::this_thread::sleep_for( std::chrono::seconds{ 3 } );
        goto l_attempt_connect;
    }
    comms() << "Connected to the controller.\n";
    
    STATUS = READY;
    DASHBOARD.begin();

    do {
        comms( EchoLevel_Input ) << "Command: ";

        while( !kbhit() ) { 
            std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } );
            if( STATUS != READY ) goto l_main_loop_break;
        }

        std::unique_lock lock{ comms.mtx() };
        std::string cmd; std::cin >> cmd;
        lock.unlock();

        if( STATUS != READY ) goto l_main_loop_break;

        if( COMMAND.exec( cmd ) ) 
            comms( EchoLevel_Ok ) << "Command \"" << cmd << "\" executed.\n";
        else
            comms( EchoLevel_Error ) << "Invalid command \"" << cmd << "\".\n";

    } while( STATUS == READY );
l_main_loop_break:
    CTRL.disconnect( 0 );
    if( DASHBOARD.th.joinable() ) DASHBOARD.th.join();

    if( STATUS == RESET ) {
        goto l_attempt_connect;
    }

    return ixN::end_runtime( argc, argv, 0, nullptr, nullptr );
}